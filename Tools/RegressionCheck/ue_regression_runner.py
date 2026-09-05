"""Drives every row of a run inside the editor, from one run-in-editor.py call.

Reads Saved/Regression/run.json:

    {"run": "<id>", "rows": ["smoke-two-clients@0", ...], "fixed_step": true, "dt": 0.016666,
     "tapes": true, "screen_percentage": 50}

Arms a slate post-tick state machine and returns; the orchestrator follows the run by tailing
the log for the REGRESSION markers each row emits:

    REGRESSION BEGIN <rid> run=<run> latency=<ms> loss=<pct> idx=<n> game=<t>
    REGRESSION ROLES <rid> <role>=<world>:<player id> ...
    REGRESSION INJECT <rid> frame=<f> <role> <action> <press|release>
    REGRESSION MARK <rid> <text>
    REGRESSION END <rid> status=<ok|error> frames=<n> game=<s> pie=<s>
    REGRESSION DONE run=<run> [status=stopped|invalid]

Worlds: a play session started as a client with N clients under one process holds the dedicated
server as PIE instance 0 and client k as instance k; each is addressed by its package path. A
role lives in one client world, drives that world's player controller, and is matched to its
server pawn by the replicated player id, never by name.

Frames: the runner's frame is the server world's game time since the row's BEGIN, in sixtieths.
A plan step is due when that frame reaches the step's; the INJECT marker carries it, and the
evaluator pairs it with the INPUT line's shared simulation frame.

Latency: the row's round trip is split evenly, NetEmulation.PktLag on every world; loss is
NetEmulation.PktLoss on every world; both are cleared with NetEmulation.Off at the row's end.

Anything thrown releases every hold, restores the clock and screen percentage, ends play, and
the run continues with the next row.
"""
import importlib
import json
import os
import re
import sys
import time
import traceback
import types

import unreal

HERE = os.path.dirname(os.path.abspath(__file__))
if HERE not in sys.path:
    sys.path.insert(0, HERE)
import scenarios as SC  # noqa: E402

# The editor keeps sys.modules between run-in-editor.py calls; reload so an edited scenarios.py
# runs at its current revision.
SC = importlib.reload(SC)

# Survives that reload, and holds the running callback so a later call can cancel it.
STATE = sys.modules.setdefault("_fm_regression_state", types.ModuleType("_fm_regression_state"))

PROJ = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
REG_DIR = os.path.join(PROJ, "Saved", "Regression")
LOG_PATH = os.path.join(PROJ, "Saved", "Logs", "Fathom.log")
STOP_FILE = os.path.join(REG_DIR, "stop")

WORLD_TIMEOUT_S = 30.0
UNTIL_TIMEOUT_S = 120.0
SETTLE_FRAMES = 30
TRACE_RE = re.compile(r"LogFMTrace: \[(\d+)\] \[(S|C\d+)\] ([A-Z][A-Z ]*?)(?: (.*))?$")

les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
ues = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)


def key(name):
    k = unreal.Key()
    k.import_text(name)
    return k


# --- key resolution -------------------------------------------------------------
# Actions are named in plans; the keys come from the mapping contexts in scenarios.IMC_PATHS, so
# a rebind moves the fixture with the game. The move action's WASD quartet is kept separately.
_ACTION_KEYS = {}
_MOVE_KEYS = {}


def resolve_keys():
    _ACTION_KEYS.clear()
    _MOVE_KEYS.clear()
    for path in SC.IMC_PATHS:
        imc = unreal.load_asset(path)
        if not imc:
            continue
        for m in imc.get_editor_property("default_key_mappings").get_editor_property("mappings"):
            action = m.get_editor_property("action")
            if not action:
                continue
            name = action.get_name()
            kname = m.get_editor_property("key").export_text()
            short = name[3:].lower() if name.startswith("IA_") else name.lower()
            if short == "move":
                if kname in ("W", "A", "S", "D"):
                    _MOVE_KEYS[kname] = kname
                continue
            _ACTION_KEYS.setdefault(short, kname)
    return _ACTION_KEYS


def key_for(action):
    return _ACTION_KEYS.get(action)


def move_keys(x, y):
    """The WASD keys whose sum is the requested direction: X right, Y forward."""
    out = []
    if y > 0:
        out.append("W")
    elif y < 0:
        out.append("S")
    if x > 0:
        out.append("D")
    elif x < 0:
        out.append("A")
    return [k for k in out if k in _MOVE_KEYS]


# --- worlds ---------------------------------------------------------------------

def pie_world(instance):
    """The PIE world of one instance, by package path, or None before it exists."""
    editor_world = ues.get_editor_world()
    pkg = editor_world.get_outer().get_name()
    folder, name = pkg.rsplit("/", 1)
    return unreal.find_object(None, "%s/UEDPIE_%d_%s.%s" % (folder, instance, name, name))


def world_for(tag):
    return pie_world(0 if tag == "S" else int(tag[1:]))


# --- the run --------------------------------------------------------------------

class Run(object):
    def __init__(self, cfg):
        self.cfg = cfg
        self.run_id = cfg["run"]
        self.rows = list(cfg["rows"])
        self.dt = float(cfg.get("dt", 1.0 / 60.0))
        self.fixed = bool(cfg.get("fixed_step", True))
        self.tapes = bool(cfg.get("tapes", True))
        self.screen_pct = cfg.get("screen_percentage")
        self.out_dir = os.path.join(REG_DIR, self.run_id)
        os.makedirs(self.out_dir, exist_ok=True)

        self.idx = -1
        self.rid = self.sid = None
        self.latency = 0
        self.phase = "next"
        self.wait = 0
        self.phase_wall_t0 = time.time()
        self.frame = 0
        self.step = 0
        self.holds = []          # (world tag, key name) currently down
        self.pending = []        # (frame due, world tag, key name, role, action)
        self.tape = None
        self.worlds = {}         # tag -> world
        self.pcs = {}            # role -> that client's player controller
        self.pawns = {}          # (role, world tag) -> pawn
        self.handle = None
        self.orig_screen_pct = None
        self.errors = 0
        self.begin_game_time = 0.0
        self.last_sampled = -1
        self.until_tag, self.until_need, self.until_count = None, 0, 0
        self.log_offset, self.log_partial, self.last_until_frame = 0, "", -1
        self.pie_wall = 0.0
        self.settle_at = None

    # -- lifecycle ----------------------------------------------------------------
    def mark(self, text):
        unreal.log("REGRESSION " + text)
        unreal.log_flush()

    def arm(self):
        resolve_keys()
        problems = SC.validate(resolve_action=key_for)
        if problems:
            for p in problems:
                unreal.log_error("REGRESSION VALIDATE " + p)
            self.mark("DONE run=%s status=invalid" % self.run_id)
            return False
        self.handle = unreal.register_slate_post_tick_callback(self.tick)
        return True

    def finish(self, stopped=False):
        self.release_all()
        if self.handle is not None:
            unreal.unregister_slate_post_tick_callback(self.handle)
            self.handle = None
        try:
            if self.fixed:
                unreal.FMTimeTools.set_fixed_time_step(False)
            if self.orig_screen_pct is not None:
                unreal.SystemLibrary.execute_console_command(
                    None, "r.ScreenPercentage %d" % self.orig_screen_pct)
        except Exception:
            pass
        self.mark("DONE run=%s%s" % (self.run_id, " status=stopped" if stopped else ""))

    def release_all(self):
        for wtag, kname in list(self.holds):
            try:
                pc = self.pc_in(wtag)
                if pc:
                    unreal.FMInputTools.input_key(pc, key(kname), False)
            except Exception:
                pass
        self.holds = []
        self.pending = []

    # -- the state machine --------------------------------------------------------
    def tick(self, delta):
        try:
            getattr(self, "phase_" + self.phase)()
        except Exception as exc:
            unreal.log_error("REGRESSION ERROR %s: %s" % (self.rid, exc))
            unreal.log_error(traceback.format_exc())
            self.mark("END %s status=error msg=%s" % (self.rid, str(exc)[:120]))
            try:
                self.release_all()
                self.close_tape()
                self.emulation_off()
                if les.is_in_play_in_editor():
                    les.editor_request_end_play()
            except Exception:
                pass
            self.errors += 1
            if self.errors > len(self.rows) + 2:
                self.finish()
                return
            self.goto("aborting")

    def goto(self, phase):
        self.phase, self.wait = phase, 0
        self.phase_wall_t0 = time.time()

    def phase_aborting(self):
        self.wait += 1
        if les.is_in_play_in_editor():
            if self.wait % 120 == 1:
                les.editor_request_end_play()
            return
        self.goto("next")

    def phase_next(self):
        self.idx += 1
        if self.idx >= len(self.rows):
            self.finish()
            return
        if os.path.exists(STOP_FILE):
            self.finish(stopped=True)
            return
        self.rid = self.rows[self.idx]
        self.sid, self.latency = SC.split_run_id(self.rid)
        self.goto("stopping")

    def phase_stopping(self):
        self.wait += 1
        if les.is_in_play_in_editor():
            if self.wait % 120 == 1:
                les.editor_request_end_play()
            return
        self.goto("apply")

    def phase_apply(self):
        s = SC.SCENARIOS[self.sid]
        for name, value in s.get("cvars", {}).items():
            unreal.SystemLibrary.execute_console_command(None, "%s %s" % (name, value))
        if self.fixed:
            unreal.FMTimeTools.set_fixed_time_step(True, self.dt)
        if self.screen_pct and self.orig_screen_pct is None:
            self.orig_screen_pct = unreal.SystemLibrary.get_console_variable_int_value(
                "r.ScreenPercentage") or 100
            unreal.SystemLibrary.execute_console_command(
                None, "r.ScreenPercentage %d" % int(self.screen_pct))
        self.pie_wall = time.time()
        les.editor_request_begin_play()
        self.goto("wait_world")

    def phase_wait_world(self):
        """Every world the row names exists, every role's client has a controlled pawn, and the
        server holds a pawn with that role's player id."""
        s = SC.SCENARIOS[self.sid]
        self.wait += 1
        timed_out = time.time() - self.phase_wall_t0 > WORLD_TIMEOUT_S
        if not les.is_in_play_in_editor():
            if timed_out:
                raise RuntimeError("PIE did not start")
            return
        worlds = dict((w, world_for(w)) for w in s["worlds"])
        missing = [w for w, obj in worlds.items() if obj is None]
        if missing:
            if timed_out:
                raise RuntimeError("no world for %s after %.0fs" % (", ".join(missing), WORLD_TIMEOUT_S))
            return
        pcs, pawns = {}, {}
        for role, (wtag, _loc, _yaw) in s["roles"].items():
            pc = unreal.GameplayStatics.get_player_controller(worlds[wtag], 0)
            pawn = pc.get_controlled_pawn() if pc else None
            if pawn is None:
                if timed_out:
                    raise RuntimeError("role %s has no pawn in %s after %.0fs" % (role, wtag, WORLD_TIMEOUT_S))
                return
            pid = self.player_id(pc.get_editor_property("player_state"))
            server_pawn = self.server_pawn_for(worlds["S"], pid)
            if server_pawn is None:
                if timed_out:
                    raise RuntimeError("role %s (player %s) has no server pawn" % (role, pid))
                return
            pcs[role] = pc
            pawns[(role, wtag)] = pawn
            pawns[(role, "S")] = server_pawn
        self.worlds, self.pcs, self.pawns = worlds, pcs, pawns
        self.goto("setup")

    @staticmethod
    def player_id(player_state):
        return int(player_state.get_player_id()) if player_state else -1

    def server_pawn_for(self, server_world, pid):
        for pawn in unreal.GameplayStatics.get_all_actors_of_class(server_world, unreal.Pawn):
            if self.player_id(pawn.get_player_state()) == pid:
                return pawn
        return None

    def phase_setup(self):
        s = SC.SCENARIOS[self.sid]
        self.apply_emulation(self.latency, float(s.get("loss", 0.0)))
        for role, (wtag, loc, yaw) in s["roles"].items():
            self.pawns[(role, "S")].set_actor_location_and_rotation(
                unreal.Vector(*loc), unreal.Rotator(0.0, 0.0, yaw), False, True)
            self.pcs[role].set_control_rotation(unreal.Rotator(0.0, 0.0, yaw))
        server = self.worlds["S"]
        self.begin_game_time = unreal.GameplayStatics.get_time_seconds(server)
        self.mark("BEGIN %s run=%s latency=%d loss=%s idx=%d game=%.3f"
                  % (self.rid, self.run_id, self.latency, s.get("loss", 0.0), self.idx, self.begin_game_time))
        self.mark("ROLES %s %s" % (self.rid, " ".join(
            "%s=%s:%d" % (role, wtag, self.player_id(self.pcs[role].get_editor_property("player_state")))
            for role, (wtag, _l, _y) in sorted(s["roles"].items()))))
        if self.tapes:
            self.tape = open(os.path.join(self.out_dir, "%s.tape.tsv" % self.rid), "w")
            self.tape.write("frame\tworld\trole\tx\ty\tz\tyaw\n")
        self.frame, self.step, self.pending, self.holds, self.last_sampled = 0, 0, [], [], -1
        stop = s.get("stop", {})
        self.until_tag, self.until_need, self.until_count = None, 0, 0
        if "until" in stop:
            self.until_tag, self.until_need = stop["until"]
        unreal.log_flush()
        self.log_offset, self.log_partial, self.last_until_frame = os.path.getsize(LOG_PATH), "", -1
        self.settle_at = None
        self.goto("run")

    def apply_emulation(self, round_trip_ms, loss_pct):
        for world in self.worlds.values():
            unreal.SystemLibrary.execute_console_command(world, "NetEmulation.PktLag %d" % (round_trip_ms // 2))
            unreal.SystemLibrary.execute_console_command(world, "NetEmulation.PktLoss %d" % int(loss_pct))

    def emulation_off(self):
        for world in self.worlds.values():
            try:
                unreal.SystemLibrary.execute_console_command(world, "NetEmulation.Off")
            except Exception:
                pass

    # -- running a row ------------------------------------------------------------
    def now(self):
        return unreal.GameplayStatics.get_time_seconds(self.worlds["S"])

    def phase_run(self):
        s = SC.SCENARIOS[self.sid]
        now = self.now()
        elapsed = now - self.begin_game_time
        f = self.frame = int(round(elapsed * 60.0))
        plan = s.get("plan", [])
        while self.step < len(plan) and plan[self.step][0] <= f:
            self.do_op(plan[self.step])
            self.step += 1
        self.expire_holds(f)
        self.sample_if_due(f)
        stop = s.get("stop", {})
        if "duration" in stop and elapsed >= float(stop["duration"]):
            self.goto("settle")
            return
        if self.until_tag:
            if f % 6 == 0 and f != self.last_until_frame:
                self.last_until_frame = f
                self.count_trace()
            if self.until_count >= self.until_need or elapsed >= float(stop.get("timeout", UNTIL_TIMEOUT_S)):
                self.goto("settle")

    def count_trace(self):
        """Lines of the until tag written since BEGIN, read off the flushed log from where the
        last read stopped, a partial trailing line carried to the next read."""
        unreal.log_flush()
        try:
            size = os.path.getsize(LOG_PATH)
        except OSError:
            return
        if size <= self.log_offset:
            return
        with open(LOG_PATH, "rb") as fh:
            fh.seek(self.log_offset)
            data = fh.read(size - self.log_offset)
        self.log_offset = size
        lines = (self.log_partial + data.decode("utf-8", "replace")).split("\n")
        self.log_partial = lines.pop()
        for line in lines:
            m = TRACE_RE.search(line)
            if m and m.group(3).strip() == self.until_tag:
                self.until_count += 1

    def pc_in(self, wtag):
        for role, (rw, _l, _y) in SC.SCENARIOS[self.sid]["roles"].items():
            if rw == wtag:
                return self.pcs.get(role)
        return None

    def do_op(self, stepv):
        _frame, role, op = stepv[0], stepv[1], stepv[2]
        wtag = SC.SCENARIOS[self.sid]["roles"][role][0]
        pc = self.pcs[role]
        if op in ("tap", "press", "release", "hold"):
            action = stepv[3]
            kname = key_for(action)
            if op == "release":
                self.up(wtag, kname, role, action)
                return
            unreal.FMInputTools.input_key(pc, key(kname), True)
            self.holds.append((wtag, kname))
            self.mark("INJECT %s frame=%d %s %s press" % (self.rid, self.frame, role, action))
            if op == "tap":
                self.pending.append((self.frame + 2, wtag, kname, role, action))
            elif op == "hold":
                self.pending.append((self.frame + int(stepv[4]), wtag, kname, role, action))
        elif op == "move":
            x, y = float(stepv[3]), float(stepv[4])
            frames = int(stepv[5]) if len(stepv) > 5 else 0
            for kname in move_keys(x, y):
                if (wtag, kname) not in self.holds:
                    unreal.FMInputTools.input_key(pc, key(kname), True)
                    self.holds.append((wtag, kname))
                    self.mark("INJECT %s frame=%d %s move-%s press" % (self.rid, self.frame, role, kname))
                if frames > 0:
                    self.pending.append((self.frame + frames, wtag, kname, role, "move-" + kname))
        elif op == "stop_move":
            for w, kname in list(self.holds):
                if w == wtag and kname in _MOVE_KEYS:
                    self.up(wtag, kname, role, "move-" + kname)
        elif op == "face":
            pc.set_control_rotation(unreal.Rotator(0.0, 0.0, float(stepv[3])))
        elif op == "teleport":
            pawn = self.pawns[(role, "S")]
            loc = stepv[3]
            yaw = float(stepv[4]) if len(stepv) > 4 else pawn.get_actor_rotation().yaw
            pawn.set_actor_location_and_rotation(
                unreal.Vector(*loc), unreal.Rotator(0.0, 0.0, yaw), False, True)
        elif op == "mark":
            self.mark("MARK %s %s" % (self.rid, stepv[3]))
        else:
            raise RuntimeError("unknown plan op %r" % (op,))

    def up(self, wtag, kname, role, action):
        if (wtag, kname) in self.holds:
            unreal.FMInputTools.input_key(self.pcs[role], key(kname), False)
            self.holds.remove((wtag, kname))
            self.mark("INJECT %s frame=%d %s %s release" % (self.rid, self.frame, role, action))

    def expire_holds(self, f):
        still = []
        for due, wtag, kname, role, action in self.pending:
            if f >= due:
                self.up(wtag, kname, role, action)
            else:
                still.append((due, wtag, kname, role, action))
        self.pending = still

    def sample_if_due(self, f):
        every = int(SC.SCENARIOS[self.sid].get("tape_every", 2))
        if self.tapes and self.tape and f % every == 0 and f != self.last_sampled:
            self.last_sampled = f
            for (role, wtag), pawn in sorted(self.pawns.items()):
                loc = pawn.get_actor_location()
                self.tape.write("%d\t%s\t%s\t%.1f\t%.1f\t%.1f\t%.1f\n" % (
                    f, wtag, role, loc.x, loc.y, loc.z, pawn.get_actor_rotation().yaw))

    def close_tape(self):
        if self.tape:
            self.tape.close()
            self.tape = None

    def phase_settle(self):
        """Every hold released, then SETTLE_FRAMES of quiet before the row ends."""
        if self.settle_at is None:
            self.release_all()
            self.settle_at = self.frame
        f = self.frame = int(round((self.now() - self.begin_game_time) * 60.0))
        self.sample_if_due(f)
        if f - self.settle_at < SETTLE_FRAMES:
            return
        self.close_tape()
        self.emulation_off()
        self._end = (self.now() - self.begin_game_time, f)
        self.pie_wall_end = time.time()
        les.editor_request_end_play()
        self.goto("ending")

    def phase_ending(self):
        self.wait += 1
        if les.is_in_play_in_editor():
            if self.wait % 120 == 1:
                les.editor_request_end_play()
            return
        game_s, frames = getattr(self, "_end", (0.0, 0))
        pie = (self.pie_wall_end - self.pie_wall) + (time.time() - self.pie_wall_end)
        self.mark("END %s status=ok frames=%d game=%.3f pie=%.1f" % (self.rid, frames, game_s, pie))
        self.goto("next")


def main():
    with open(os.path.join(REG_DIR, "run.json")) as fh:
        cfg = json.load(fh)
    prior = getattr(STATE, "ACTIVE_RUN", None)
    if prior is not None:
        prior.finish()
    run = Run(cfg)
    STATE.ACTIVE_RUN = run
    if run.arm():
        print("ARMED %s: %d row(s)" % (run.run_id, len(run.rows)))
    print("DONE")


main()

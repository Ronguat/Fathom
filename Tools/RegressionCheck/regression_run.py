"""Drives a regression run end to end: preflight, the editor-side runner, then evaluation.

Run through regression-run.sh, which is the documented entry point.

    regression-run.sh [--all | <id>... | --family <name>] [--latency 0,100] [--no-mutate]
                      [--dry-run] [--run <id>] [--resume <id>] [--stop] [--accept-golden]

It never drives PIE itself. ue_regression_runner.py does that from inside the editor; this waits
on the REGRESSION markers it emits, saves each row's slice, and evaluates it: the row's own
assertions, the universal set, the cost readout, the golden diff, then the mutations. A row whose
END has not arrived by 3x its duration or timeout + 60 s is abandoned and marked TIMEOUT.
"""
import argparse
import json
import os
import subprocess
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import paths  # noqa: E402
import scenarios as SC  # noqa: E402

PY = paths.ENGINE_PY
ROOT = paths.ROOT
RUNNER = os.path.join(HERE, "ue_regression_runner.py")
PREFLIGHT = os.path.join(HERE, "regression_preflight.py")
EVAL = os.path.join(HERE, "regression_eval.py")
ROWS = os.path.join(HERE, "regression_rows.py")
STOP_FILE = os.path.join(paths.REG, "stop")
LOCK = os.path.join(paths.REG, ".lock")


def binary_stamp():
    """The game module's modification time, the mark a resume must match."""
    try:
        return int(os.path.getmtime(paths.DLL))
    except OSError:
        return 0


def sh(cmd, **kw):
    return subprocess.run(cmd, capture_output=True, text=True, cwd=ROOT, **kw)


def in_editor(statement):
    return sh([PY, paths.RUN_IN_EDITOR, "-c", statement], timeout=60)


# --- preflight ----------------------------------------------------------------

PLAY_SETTINGS = ("import unreal; s = unreal.get_default_object(unreal.LevelEditorPlaySettings); "
                 "print('PLAY', s.get_editor_property('play_net_mode'), "
                 "s.get_editor_property('play_number_of_clients'), "
                 "s.get_editor_property('run_under_one_process'))")


def play_settings_from_ini():
    out = {}
    if os.path.exists(paths.USER_INI):
        for line in open(paths.USER_INI, errors="ignore"):
            for k in ("PlayNetMode", "PlayNumberOfClients", "RunUnderOneProcess"):
                if line.startswith(k + "="):
                    out[k] = line.split("=", 1)[1].strip()
    return out


def preflight(args):
    """Everything that makes a run's result meaningless if wrong, checked before PIE."""
    problems = []
    print("  preflight")

    r = in_editor("import unreal; print('PIE', "
                  "unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).is_in_play_in_editor())")
    if "RESULT" not in r.stdout:
        problems.append("the editor did not answer -- is it open?")
    elif "PIE True" in r.stdout:
        problems.append("a play session is already running; stop it first")
    else:
        print("    editor answers, not in PIE")

    # The two-world requirement: a dedicated server and at least two clients under one process.
    r = in_editor(PLAY_SETTINGS)
    live = [l for l in r.stdout.splitlines() if "PLAY " in l]
    if live:
        toks = live[-1].split("PLAY ", 1)[1].split()
        mode, clients, one_proc = toks[0], toks[1], toks[2]
        source = "live"
    else:
        ini = play_settings_from_ini()
        mode, clients, one_proc = ini.get("PlayNetMode", "?"), ini.get("PlayNumberOfClients", "1"), ini.get("RunUnderOneProcess", "True")
        source = "ini"
    if "CLIENT" not in mode.upper() or "STANDALONE" in mode.upper():
        problems.append("PlayNetMode is %s (%s); the loop needs Play As Client with a dedicated server" % (mode, source))
    if int(clients or 1) < 2:
        problems.append("PlayNumberOfClients is %s (%s); the loop needs at least two" % (clients, source))
    if "TRUE" not in one_proc.upper():
        problems.append("RunUnderOneProcess is %s (%s); the runner addresses worlds in one process" % (one_proc, source))
    if not problems:
        print("    play settings: %s, %s clients, one process (%s)" % (mode, clients, source))

    r = sh([PY, PREFLIGHT])
    if r.returncode != 0:
        problems.append("offline preflight failed; run regression_preflight.py to see why")
    else:
        print("    offline preflight green")

    g = sh(["git", "status", "--short"])
    if g.stdout.strip():
        print("    working tree, at the run's start:")
        for line in g.stdout.splitlines():
            print("      " + line)
    else:
        print("    working tree clean")
    return problems


def take_lock():
    """A second run against the same editor would interleave two PIE sessions in one log."""
    try:
        os.makedirs(LOCK)
    except OSError:
        pid_file = os.path.join(LOCK, "pid")
        pid = open(pid_file).read().strip() if os.path.exists(pid_file) else "?"
        alive = sh(["tasklist", "/FI", "PID eq %s" % pid]).stdout
        if pid and pid in alive:
            print("  another run holds the lock (pid %s)" % pid)
            return False
        print("  reclaiming a lock left by dead pid %s" % pid)
    with open(os.path.join(LOCK, "pid"), "w") as fh:
        fh.write(str(os.getpid()))
    return True


def release_lock():
    try:
        os.remove(os.path.join(LOCK, "pid"))
        os.rmdir(LOCK)
    except OSError:
        pass


# --- following the run ----------------------------------------------------------

def tail_new(path, offset):
    size = os.path.getsize(path)
    if size < offset:
        offset = 0
    with open(path, "rb") as fh:
        fh.seek(offset)
        data = fh.read()
    return data.decode("utf-8", "replace"), offset + len(data)


def save_slice(run, rid):
    """The raw log between this row's markers, kept beside its tape. The last BEGIN wins."""
    out_dir = os.path.join(paths.REG, run)
    os.makedirs(out_dir, exist_ok=True)
    begin = "REGRESSION BEGIN %s run=%s " % (rid, run)
    end = "REGRESSION END %s " % rid
    keep, started, done = [], False, []
    for line in open(paths.LOG, errors="ignore"):
        if begin in line:
            keep, started = [], True
        if started:
            keep.append(line)
            if end in line:
                done, started = keep, False
    keep = done or keep
    path = os.path.join(out_dir, "%s.slice.log" % rid)
    with open(path, "w", errors="replace") as fh:
        fh.writelines(keep)
    return path


def row_eval(rid, slice_path):
    tape = slice_path.replace(".slice.log", ".tape.tsv")
    return sh([PY, ROWS, rid, slice_path, "--tape", tape])


def evaluate(run, rid, slice_path, args):
    """The row's assertions, the universal set, the cost readout, the golden diff, then its
    mutations. The mutations say the assertions could have failed."""
    sid, _ms = SC.split_run_id(rid)
    s = SC.SCENARIOS[sid]
    out = dict(rc=0, detail="", universal="", cost="", golden="", mutations="")

    r = row_eval(rid, slice_path)
    tail = [ln for ln in r.stdout.splitlines() if "passed," in ln]
    out["rc"] = r.returncode
    out["detail"] = tail[-1].strip() if tail else (r.stdout or r.stderr).strip()[-120:]

    allow = ",".join(s.get("allow", []))
    u = sh([PY, EVAL, slice_path, "--universal", "--allow", allow])
    bad = [ln.strip() for ln in u.stdout.splitlines() if ln.strip().startswith("FAIL")]
    out["universal"] = "clean" if u.returncode == 0 else "; ".join(bad)
    if u.returncode != 0:
        out["rc"] = out["rc"] or 1

    c = sh([PY, EVAL, slice_path, "--cost"])
    out["cost"] = c.stdout.strip()

    keep = ",".join(s.get("golden", {}).get("keep", []))
    g = sh([PY, EVAL, slice_path, "--golden", "--id", rid, "--keep", keep]
           + (["--accept"] if args.accept_golden else []))
    out["golden"] = g.stdout.strip()

    if args.no_mutate or out["rc"] not in (0, None):
        out["mutations"] = "skipped"
    else:
        out["mutations"] = prove_mutations(rid, slice_path, s, allow)
        if out["mutations"].startswith("UNPROVEN"):
            out["rc"] = 1
    return out


def prove_mutations(rid, slice_path, s, allow):
    """Each mutation must turn the row or the universal set red."""
    proven, unproven = 0, []
    for mut in s.get("mutations", []):
        dst = slice_path.replace(".slice.log", ".mutated.log")
        spec = ":".join(str(x) for x in mut)
        m = sh([PY, EVAL, slice_path, "--mutate", spec, "--out", dst])
        if m.returncode != 0:
            unproven.append("%s (could not apply)" % spec)
            continue
        red = row_eval(rid, dst).returncode != 0
        if not red:
            red = sh([PY, EVAL, dst, "--universal", "--allow", allow]).returncode != 0
        if red:
            proven += 1
        else:
            unproven.append(spec)
        try:
            os.remove(dst)
        except OSError:
            pass
    if unproven:
        return "UNPROVEN: " + "; ".join(unproven)
    return "%d proven" % proven


def append_history(run, results):
    """One line per row per run, so a band can be watched drifting across runs."""
    path = os.path.join(paths.REG, "history.tsv")
    cols = ("run", "stamp", "id", "status", "rc", "frames", "detail", "universal", "cost", "golden", "mutations")
    tab, nl = chr(9), chr(10)
    fresh = not os.path.exists(path)
    with open(path, "a") as fh:
        if fresh:
            fh.write(tab.join(cols) + nl)
        for r in results:
            fh.write(tab.join(str(r.get(k, "")).replace(tab, " ") for k in cols) + nl)


def main():
    try:
        sys.stdout.reconfigure(line_buffering=True)
    except AttributeError:
        pass
    ap = argparse.ArgumentParser()
    ap.add_argument("ids", nargs="*")
    ap.add_argument("--all", action="store_true")
    ap.add_argument("--family")
    ap.add_argument("--latency", default=None, help="run only these round trips, ms, comma-separated")
    ap.add_argument("--no-mutate", action="store_true")
    ap.add_argument("--accept-golden", action="store_true")
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--run", default=None)
    ap.add_argument("--screen-percentage", type=int, default=50,
                    help="render scale for the run; nothing the loop reads is rendered")
    ap.add_argument("--stop", action="store_true", help="ask the run in progress to end after its row")
    ap.add_argument("--resume", default=None, metavar="RUN",
                    help="run the rows of RUN that have no slice yet, on the same binary")
    a = ap.parse_args()

    if a.stop:
        os.makedirs(paths.REG, exist_ok=True)
        with open(STOP_FILE, "w") as fh:
            fh.write(time.strftime("%Y-%m-%d %H:%M:%S"))
        print("  stop requested; the run ends after its current row")
        return 0

    rows = None
    if a.resume:
        run_dir = os.path.join(paths.REG, a.resume)
        try:
            with open(os.path.join(run_dir, "run.json")) as fh:
                saved = json.load(fh)
        except (OSError, ValueError):
            print("no resumable run at %s" % run_dir)
            return 2
        if saved.get("binary") != binary_stamp():
            print("  %s ran on a different binary; its rows are stale, start a new run" % a.resume)
            return 2
        rows = [r for r in saved["rows"] if not os.path.exists(os.path.join(run_dir, r + ".slice.log"))]
        if not rows:
            print("  %s has a slice for every row; nothing to resume" % a.resume)
            return 0
        print("  resuming %s: %d of %d row(s) remain" % (a.resume, len(rows), len(saved["rows"])))
        a.run = a.resume

    shape = SC.validate()
    if shape:
        for p in shape:
            print("  scenarios.py: " + p)
        return 2

    if rows is None:
        if a.all:
            ids = sorted(SC.SCENARIOS)
        elif a.family:
            ids = SC.by_family(a.family)
        else:
            ids = a.ids
        if not ids:
            print("nothing selected; pass ids, --family <name> or --all")
            return 2
        unknown = [i for i in ids if i not in SC.SCENARIOS]
        if unknown:
            print("unknown scenario(s): %s" % ", ".join(unknown))
            return 2
        want = [int(x) for x in a.latency.split(",")] if a.latency else None
        rows = [SC.run_id(sid, ms) for sid, ms in SC.runs_for(ids) if want is None or ms in want]
        if not rows:
            print("no row runs at the latencies asked for")
            return 2

    run = a.run or time.strftime("%m%d-%H%M%S")
    print()
    print("  regression run %s: %d row(s), fixed 1/60" % (run, len(rows)))
    print()
    problems = preflight(a)
    if problems:
        print()
        for p in problems:
            print("  BLOCKED  " + p)
        return 2
    if a.dry_run:
        print("\n  dry run: %s" % ", ".join(rows))
        return 0
    if not take_lock():
        return 2
    try:
        return drive(run, rows, a)
    finally:
        release_lock()


def drive(run, rows, a):
    os.makedirs(paths.REG, exist_ok=True)
    if os.path.exists(STOP_FILE):
        os.remove(STOP_FILE)
    cfg = dict(run=run, rows=rows, fixed_step=True, dt=1.0 / 60.0, tapes=True,
               screen_percentage=a.screen_percentage, binary=binary_stamp())
    with open(os.path.join(paths.REG, "run.json"), "w") as fh:
        json.dump(cfg, fh)
    run_dir = os.path.join(paths.REG, run)
    os.makedirs(run_dir, exist_ok=True)
    earlier = []
    if os.path.exists(os.path.join(run_dir, "run.json")):
        with open(os.path.join(run_dir, "run.json")) as fh:
            earlier = json.load(fh).get("rows", [])
    with open(os.path.join(run_dir, "run.json"), "w") as fh:
        json.dump(dict(cfg, rows=sorted(set(earlier) | set(rows))), fh)

    offset = os.path.getsize(paths.LOG)
    r = sh([PY, paths.RUN_IN_EDITOR, RUNNER], timeout=120)
    if "ARMED" not in r.stdout:
        print("  the runner did not arm:")
        print(r.stdout[-800:] or r.stderr[-800:])
        return 2
    print("  armed; following the run")
    print()

    budget = {}
    for rid in rows:
        stop = SC.SCENARIOS[SC.split_run_id(rid)[0]].get("stop", {})
        budget[rid] = 3 * float(stop.get("duration", stop.get("timeout", 60))) + 60.0

    results, seen, started_at = [], set(), time.time()
    pending = list(rows)
    deadline = started_at + sum(budget.values()) + 120.0
    stopped = False
    while pending and time.time() < deadline and not stopped:
        time.sleep(2.0)
        chunk, offset = tail_new(paths.LOG, offset)
        for line in chunk.splitlines():
            if "REGRESSION DONE " in line and "status=stopped" in line:
                stopped = True
                continue
            if "REGRESSION END " not in line:
                continue
            rid = line.split("REGRESSION END ", 1)[1].split()[0]
            if rid in seen or rid not in pending:
                continue
            seen.add(rid)
            pending.remove(rid)
            status = "ok" if "status=ok" in line else "error"
            frames = "-"
            for tok in line.split():
                if tok.startswith("frames="):
                    frames = tok[7:]
            path = save_slice(run, rid)
            ev = (dict(rc=1, detail="runner reported an error", universal="-", cost="-",
                       golden="-", mutations="-") if status != "ok"
                  else evaluate(run, rid, path, a))
            results.append(dict(run=run, stamp=time.strftime("%Y-%m-%d %H:%M:%S"),
                                id=rid, status=status, frames=frames, **ev))
            print("    %-26s %-6s %-22s univ=%-8s %s | %s" % (
                rid, status, ev["detail"][:22], ev["universal"][:8], ev["golden"][:26], ev["mutations"][:18]))
            print("      cost: %s" % ev["cost"])
    if stopped:
        if os.path.exists(STOP_FILE):
            os.remove(STOP_FILE)
        print("    stopped after the current row; %d row(s) not run" % len(pending))
        print("    resume with: regression-run.sh --resume %s" % run)
    else:
        for rid in pending:
            results.append(dict(run=run, stamp=time.strftime("%Y-%m-%d %H:%M:%S"), id=rid,
                                status="timeout", frames="-", rc=1,
                                detail="no END marker within %.0fs" % budget[rid],
                                universal="-", cost="-", golden="-", mutations="-"))
            print("    %-26s TIMEOUT" % rid)

    append_history(run, results)
    report(run, results, time.time() - started_at)
    if stopped:
        return 3
    return 1 if any(r["status"] != "ok" or r["rc"] not in (0, None) for r in results) else 0


def report(run, results, wall):
    """The summary carries every row the run has produced across its sittings."""
    out_dir = os.path.join(paths.REG, run)
    os.makedirs(out_dir, exist_ok=True)
    path = os.path.join(out_dir, "summary.json")
    earlier, earlier_wall = [], 0.0
    if os.path.exists(path):
        try:
            with open(path) as fh:
                prev = json.load(fh)
            now_ids = set(r["id"] for r in results)
            earlier = [r for r in prev.get("rows", []) if r["id"] not in now_ids]
            earlier_wall = float(prev.get("wall_seconds", 0.0))
        except (OSError, ValueError, KeyError):
            earlier, earlier_wall = [], 0.0
    results = earlier + results
    wall += earlier_wall
    with open(path, "w") as fh:
        json.dump(dict(run=run, wall_seconds=round(wall, 1), rows=results), fh, indent=1)
    ok = sum(1 for r in results if r["status"] == "ok" and r["rc"] in (0, None))
    print()
    print("  %d of %d green, %.0fs wall" % (ok, len(results), wall))
    print("  %s" % os.path.join("Saved", "Regression", run, "summary.json"))
    print()


if __name__ == "__main__":
    sys.exit(main())

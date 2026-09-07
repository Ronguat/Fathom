"""The melee rows' measurements from a run's slices, one line per row, for the decision entry.

    melee_report.py <run id> [<run id> ...]

For each melee slice: the attack's start against the press, the effective windup, every HIT and
PARRY with its rewind depth and what the body moved since, the rewound body against the
attacker's client's rendering when the row keeps per-frame poses, and the tallies per world.
"""
import glob
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import paths  # noqa: E402
import scenarios as SC  # noqa: E402
from regression_rows import Context, role_pids, combat, hits, parries, rendered_at, command_frame  # noqa: E402


def describe(path):
    rid = os.path.basename(path).replace(".slice.log", "")
    sid, ms = SC.split_run_id(rid)
    ctx = Context(path)
    pids = role_pids(ctx)
    p1, p2 = pids.get("p1", -1), pids.get("p2", -1)
    press = ctx.first("INPUT", "C1", "edge=pressed")
    out = ["%-24s" % rid]
    server = combat(ctx, "S", p1)
    starts = [int(ln.fields.get("start", -1)) for ln in server if ln.fields.get("phase") == "windup"]
    release = [ln for ln in server if ln.fields.get("phase") == "release"]
    if press and starts:
        out.append("advance %d" % (command_frame(press) - starts[0]))
    if press and release:
        out.append("windup %d" % (release[0].frame - press.frame))
    for ln in hits(ctx, p1, p2) + parries(ctx, p1, p2):
        rf = int(ln.fields["rf"])
        item = "%s k=%d +%d f depth %d" % (ln.tag, int(ln.fields["k"]), ln.frame - press.frame if press else -1, ln.frame - rf)
        if "moved" in ln.fields:
            item += " moved %.1f" % ln.fields["moved"]
        if "margin" in ln.fields:
            item += " margin %d" % int(ln.fields["margin"])
        if "part" in ln.fields:
            item += " %s" % ln.fields["part"]
        seen = rendered_at(ctx, "C1", p2, rf)
        if seen and "tx" in ln.fields:
            item += " rendered-err %.1f" % (sum((ln.fields[k] - seen[i]) ** 2 for i, k in enumerate(("tx", "ty", "tz"))) ** 0.5)
        out.append(item)
    rollbacks = [ln.fields.get("n", 0.0) for ln in ctx.lines("ROLLBACK", "C1") if int(ln.fields.get("pid", -1)) == p1]
    out.append("rollbacks %d" % int(max(rollbacks) if rollbacks else 0))
    print(" | ".join(out))


def main():
    for run in sys.argv[1:]:
        for path in sorted(glob.glob(os.path.join(paths.REG, run, "melee.*.slice.log"))):
            describe(path)
    return 0


if __name__ == "__main__":
    sys.exit(main())

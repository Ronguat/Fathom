"""Summarises one run's slices: per row, the client's lead over the server in frames and the
mean measured lag, taken from the injection pairing and the COST lines.

    run_report.py <run id>
"""
import glob
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import paths  # noqa: E402
from regression_eval import cost, injection_deltas, read  # noqa: E402


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        return 2
    run_dir = os.path.join(paths.REG, sys.argv[1])
    print("  %-22s %-14s %-10s %-12s %s" % ("row", "lead (frames)", "lag (ms)", "in/out (B/s)", "tick (ms)"))
    for path in sorted(glob.glob(os.path.join(run_dir, "*.slice.log"))):
        rid = os.path.basename(path)[:-len(".slice.log")]
        trace, markers, _raw, _bad = read(path)
        deltas = injection_deltas(trace, markers)
        lead = "-" if not deltas else ("%d" % deltas[0] if max(deltas) == min(deltas) else "%d..%d" % (min(deltas), max(deltas)))
        c = cost(trace)
        if c:
            lag = sum(v["lag_ms"] for v in c.values()) / len(c)
            inb = sum(v["in_bps"] for v in c.values()) / len(c)
            outb = sum(v["out_bps"] for v in c.values()) / len(c)
            tick = sum(v["tick_ms"] for v in c.values()) / len(c)
            print("  %-22s %-14s %-10.0f %-12s %.2f" % (rid, lead, lag, "%.0f/%.0f" % (inb, outb), tick))
        else:
            print("  %-22s %-14s %-10s %-12s %s" % (rid, lead, "-", "-", "-"))
    return 0


if __name__ == "__main__":
    sys.exit(main())

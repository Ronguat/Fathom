"""Writes the scenario matrix and the coverage map in Docs/Debug-Instruments.md from scenarios.py.

    gen-matrix.py            rewrite both regions in place
    gen-matrix.py --check    exit 1 if either region is stale, printing nothing else

scenarios.py is the authority and this renders it between the region markers; docs-check runs
--check so a fixture edit that forgets the doc fails there.
"""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import paths  # noqa: E402
import scenarios as SC  # noqa: E402

DOC = os.path.join(paths.ROOT, "Docs", "Debug-Instruments.md")
BEGIN, END = "<!-- matrix:begin -->", "<!-- matrix:end -->"
COV_BEGIN, COV_END = "<!-- coverage:begin -->", "<!-- coverage:end -->"


def step_text(step):
    frame, role, op = step[0], step[1], step[2]
    rest = " ".join(str(x) for x in step[3:])
    return "f%d %s %s %s" % (frame, role, op, rest)


def plan_summary(s):
    plan = s.get("plan") or []
    if not plan:
        return "-"
    shown = "; ".join(step_text(st) for st in plan[:4])
    if len(plan) > 4:
        shown += "; +%d more" % (len(plan) - 4)
    return shown


def stop_summary(s):
    stop = s["stop"]
    if "until" in stop:
        return "%dx %s, or %.0f s" % (stop["until"][1], stop["until"][0], stop["timeout"])
    return "%.0f s" % stop["duration"]


def render():
    lines = [BEGIN, ""]
    lines.append("| Scenario | Worlds | Round trips (ms) | Plan | Stop | Covers |")
    lines.append("|---|---|---|---|---|---|")
    for fam in SC.FAMILIES:
        for sid in SC.by_family(fam):
            s = SC.SCENARIOS[sid]
            lines.append("| `%s` | %s | %s | %s | %s | %s |" % (
                sid, " ".join(s["worlds"]), ", ".join(str(l) for l in s["latencies"]),
                plan_summary(s), stop_summary(s), ", ".join(s["covers"])))
    if not SC.SCENARIOS:
        lines.append("| *none yet* | | | | | |")
    lines.append("")
    lines.append("*Generated from `Tools/RegressionCheck/scenarios.py` by "
                 "`Tools/RegressionCheck/gen-matrix.py`. Edit the fixtures there, never this table.*")
    lines.append(END)
    return "\n".join(lines)


def render_coverage():
    lines = [COV_BEGIN, ""]
    lines.append("| Mechanic | Rows asserting it |")
    lines.append("|---|---|")
    for mech in SC.MECHANICS:
        rows = sorted(sid for sid, s in SC.SCENARIOS.items() if mech in s.get("covers", []))
        lines.append("| %s | %s |" % (mech, ", ".join("`%s`" % r for r in rows) or "**none**"))
    lines.append("")
    lines.append("*Generated from each row's `covers` in `Tools/RegressionCheck/scenarios.py` by "
                 "`Tools/RegressionCheck/gen-matrix.py`.*")
    lines.append(COV_END)
    return "\n".join(lines)


def main():
    text = open(DOC, encoding="utf-8", newline="").read()
    for b, e in ((BEGIN, END), (COV_BEGIN, COV_END)):
        if b not in text or e not in text:
            print("gen-matrix: no %s region in %s" % (b, DOC))
            return 2
    stale = False
    for b, e, fresh in ((BEGIN, END, render()), (COV_BEGIN, COV_END, render_coverage())):
        i = text.index(b)
        j = text.index(e) + len(e)
        if text[i:j] != fresh:
            stale = True
            text = text[:i] + fresh + text[j:]
    if "--check" in sys.argv:
        return 1 if stale else 0
    open(DOC, "w", encoding="utf-8", newline="").write(text)
    print("gen-matrix: %d scenario(s) written" % len(SC.SCENARIOS))
    return 0


if __name__ == "__main__":
    sys.exit(main())

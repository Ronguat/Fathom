"""The checks a run's result depends on that need no editor, run before every run.

    regression_preflight.py

Shape: scenarios.py validates, and every row a scenario names has an asserting function.
Instruments: the evaluator's and the rows' self-tests pass, so each assertion has failed once.
Format lint: every trace call site in Source/Fathom opens its literal with an upper-case tag,
which is what the evaluator keys on. Exit 1 on any FAIL.
"""
import glob
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import paths  # noqa: E402
import scenarios as SC  # noqa: E402
from regression_rows import ROWS  # noqa: E402

ROWS_OUT = []
TRACE_SITE = re.compile(r'(?:FM_TRACE\s*\(\s*[^,()]+,|UE_LOG\s*\(\s*LogFMTrace\s*,\s*\w+\s*,)\s*TEXT\s*\(\s*"([^"]*)"')
# The emitter itself prints a finished line through this literal.
EMITTER_LITERAL = "%s"


def out(status, label, detail):
    ROWS_OUT.append((status, label, detail))


def shape():
    problems = SC.validate()
    for p in problems:
        out("FAIL", "scenario shape", p)
    if not problems:
        out("PASS", "scenario shape", "%d scenario(s) valid" % len(SC.SCENARIOS))
    unasserted = sorted(s for s in SC.SCENARIOS if s not in ROWS)
    if unasserted:
        out("FAIL", "every scenario has a row", "no row for: " + ", ".join(unasserted))
    elif SC.SCENARIOS:
        out("PASS", "every scenario has a row", "%d row(s)" % len(ROWS))


def instruments():
    for script in ("regression_eval.py", "regression_rows.py"):
        r = subprocess.run([paths.ENGINE_PY, os.path.join(HERE, script), "--self-test"],
                           capture_output=True, text=True)
        line = (r.stdout.strip().splitlines() or ["no output"])[-1]
        out("PASS" if r.returncode == 0 else "FAIL", script + " self-test", line)


def format_lint():
    sites, bad = 0, []
    for path in glob.glob(os.path.join(paths.ROOT, "Source", "Fathom", "**", "*.cpp"), recursive=True):
        text = open(path, errors="replace").read()
        for m in TRACE_SITE.finditer(text):
            sites += 1
            literal = m.group(1)
            if literal == EMITTER_LITERAL:
                continue
            tag = literal.split()[0] if literal.split() else ""
            if not (tag.isalpha() and tag.isupper()):
                rel = os.path.relpath(path, paths.ROOT).replace("\\", "/")
                bad.append("%s: %r opens with no tag" % (rel, literal[:40]))
    if bad:
        out("FAIL", "trace format lint", "%d offending site(s): %s" % (len(bad), "; ".join(bad[:3])))
    else:
        out("PASS", "trace format lint", "%d call site(s) open with a tag" % sites)


def main():
    shape()
    instruments()
    format_lint()
    for status, label, detail in ROWS_OUT:
        print("  %-6s %-34s %s" % (status, label, detail))
    fails = sum(1 for s, _, _ in ROWS_OUT if s == "FAIL")
    print("  %d passed, %d failed" % (len(ROWS_OUT) - fails, fails))
    return 1 if fails else 0


if __name__ == "__main__":
    sys.exit(main())

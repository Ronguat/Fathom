#!/usr/bin/env bash
# regression-run.sh -- drive a regression run: preflight, PIE, evaluation.
#
#   ./regression-run.sh --all                       # every scenario at every round trip
#   ./regression-run.sh --family harness --latency 0,100
#   ./regression-run.sh --all --dry-run             # preflight only, drives no PIE
#
# The work is in regression_run.py, on the engine's interpreter; no other Python is on this machine.
# Exit 0 = every row green, 1 = a failure or a timeout, 2 = preflight blocked, 3 = stopped.
set -uo pipefail
ENGINE_PY="/c/Program Files (x86)/UE_5.8/Engine/Binaries/ThirdParty/Python3/Win64/python.exe"
[ -x "$ENGINE_PY" ] || { echo "regression-run: no engine Python at $ENGINE_PY" >&2; exit 2; }
exec "$ENGINE_PY" "$(dirname "$0")/regression_run.py" "$@"

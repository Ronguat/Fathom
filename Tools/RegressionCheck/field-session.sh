#!/usr/bin/env bash
# field-session.sh -- one packaged client against the editor as a dedicated server, then ingest.
#
#   ./field-session.sh [seconds]      # default 20; needs Saved/Packaged/Windows/Fathom.exe
#
# Starts the editor with -server on the harness map, starts the packaged client as C1, presses the
# marker hotkey in the client window after half the time, stops both, and runs ingest_bundle.py on
# the server's newest bundle. Exit is the ingest's. The interactive editor must be closed: two
# editors would both answer the Python runner and both bind the MCP port.
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
ENGINE="/c/Program Files (x86)/UE_5.8/Engine"
ENGINE_PY="$ENGINE/Binaries/ThirdParty/Python3/Win64/python.exe"
PROJECT="C:/Users/rross/Documents/Unreal Projects/Fathom/Fathom.uproject"
CLIENT="$ROOT/Saved/Packaged/Windows/Fathom.exe"
SECONDS_TO_RUN="${1:-20}"
PORT=7777

[ -x "$CLIENT" ] || { echo "field-session: no packaged client at $CLIENT" >&2; exit 2; }
if tasklist | grep -qi "UnrealEditor.exe"; then
  echo "field-session: an editor is running; close it first" >&2; exit 2
fi

echo "  server: editor -server on L_Harness, port $PORT"
MSYS_NO_PATHCONV=1 "$ENGINE/Binaries/Win64/UnrealEditor.exe" "$PROJECT" /Game/Fathom/Maps/L_Harness -server -log -port=$PORT -unattended \
  "-ini:EditorPerProjectUserSettings:[/Script/ModelContextProtocolEngine.ModelContextProtocolSettings]:bAutoStartServer=False" \
  >/dev/null 2>&1 &
SERVER_PID=$!
powershell -NoProfile -Command "Start-Sleep -Seconds 25"

echo "  client: $CLIENT as C1"
"$CLIENT" "127.0.0.1:$PORT" -FMClient=1 -windowed -resx=800 -resy=450 >/dev/null 2>&1 &
CLIENT_PID=$!
powershell -NoProfile -Command "Start-Sleep -Seconds $((SECONDS_TO_RUN / 2))"

echo "  marker: pressing M in the client window"
powershell -NoProfile -Command "\$w = New-Object -ComObject WScript.Shell; if (\$w.AppActivate('Fathom')) { Start-Sleep -Milliseconds 500; \$w.SendKeys('m'); 'sent' } else { 'no client window' }"
powershell -NoProfile -Command "Start-Sleep -Seconds $((SECONDS_TO_RUN / 2))"

echo "  stopping the client, then the server"
taskkill //IM Fathom.exe //F >/dev/null 2>&1
powershell -NoProfile -Command "Start-Sleep -Seconds 3"
taskkill //IM UnrealEditor.exe >/dev/null 2>&1
powershell -NoProfile -Command "Wait-Process -Name UnrealEditor -Timeout 60 -ErrorAction SilentlyContinue"
tasklist | grep -qi "UnrealEditor.exe" && taskkill //IM UnrealEditor.exe //F >/dev/null 2>&1
wait $SERVER_PID $CLIENT_PID 2>/dev/null

BUNDLE=$(ls -td "$ROOT"/Saved/Fathom/Sessions/Field/*-S 2>/dev/null | head -1)
[ -n "$BUNDLE" ] || { echo "field-session: no server bundle under Saved/Fathom/Sessions/Field" >&2; exit 1; }
echo "  bundle: $BUNDLE"
exec "$ENGINE_PY" "$ROOT/Tools/RegressionCheck/ingest_bundle.py" "$BUNDLE"

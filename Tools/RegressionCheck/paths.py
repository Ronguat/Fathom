"""The paths every loop script shares: the project root, the engine's interpreter, the editor log,
the game module's DLL, the editor Python runner and the run directory."""
import os

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))
ENGINE_PY = r"C:/Program Files (x86)/UE_5.8/Engine/Binaries/ThirdParty/Python3/Win64/python.exe"
RUN_IN_EDITOR = os.path.join(ROOT, "Tools", "Editor", "run-in-editor.py")
LOG = os.path.join(ROOT, "Saved", "Logs", "Fathom.log")
DLL = os.path.join(ROOT, "Binaries", "Win64", "UnrealEditor-Fathom.dll")
REG = os.path.join(ROOT, "Saved", "Regression")
USER_INI = os.path.join(ROOT, "Saved", "Config", "WindowsEditor", "EditorPerProjectUserSettings.ini")

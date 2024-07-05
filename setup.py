from cx_Freeze import setup, Executable
import sys

build_dir = "build/sunshine-toolbox"
base = None

if sys.platform == "win32":
    base = "Win32GUI"


build_exe_options = {
    "build_exe": build_dir,
}

executables = [
    Executable("sunshine-toolbox.py", base=base)
]

setup(
    name="sunshine-toolbox",
    version="0.1",
    options={"build_exe": build_exe_options},
    executables=executables
)

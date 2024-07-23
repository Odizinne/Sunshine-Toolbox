import os
from cx_Freeze import setup, Executable

src_dir = os.path.dirname(os.path.abspath(__file__))
build_dir = "build/sunshine-toolbox"

build_exe_options = {
    "build_exe": build_dir,
}

executables = [
    Executable(
        script = os.path.join(src_dir, 'sunshine-toolbox.py'), 
        base = "Win32GUI",
        target_name = "sunshine-toolbox"
    )
]

setup(
    name="sunshine-toolbox",
    version="1.0",
    options={"build_exe": build_exe_options},
    executables=executables
)

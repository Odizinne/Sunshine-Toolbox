# Sunshine-Toolbox
[![Github All Releases](https://img.shields.io/github/downloads/odizinne/Sunshine-Toolbox/total.svg)]()
[![license](https://img.shields.io/github/license/odizinne/Sunshine-Toolbox)]()

Useful commands for sunshine automation.

## Download

You can download the latest release [here](https://github.com/Odizinne/Sunshine-Toolbox/releases/latest).

## Usage

- `--stream-on || --stream-off`

This will create or delete a file (`%APPDATA%\sunshine-status\status.txt`).  
Can be used for scripting purpose.

- `--enable hdr || --disable-hdr`

Enable / disable HDR if host support it.

- `--set-resolution "WIDTH" "HEIGHT" "REFRESH_RATE"`: 

This will set resolution from the primary monitor.  
Useful for setting client resolution as do command:  
`cmd /c "C:\path\to\sunshine-toolbox.exe" --set-resolution %SUNSHINE_CLIENT_WIDTH% %SUNSHINE_CLIENT_HEIGHT% %SUNSHINE_CLIENT_FPS%`

And to reset to default resolution as undo command:  
`cmd /c "C:\path\to\sunshine-toolbox.exe" --set-resolution 2560 1440 144`

Be sure your monitor has support for your client resolution. If not, you can add them using external tools.  
(AMD / Nvidia software, custom-resolution-utility, etc)

- `--close-bigpicture`

This will just close Steam Big Picture if the window exist.  
This allows to close Big Picture when ending the stream.

- `--bigpicture-dummyload`

This is meant to be used as main command for Steam Big Picture app.  
This script will start steam in big picture mode and end when big picture window is closed.  
This allows to close Big Picture to end the stream.


## Documentation

Use --help or -h to get documentation.

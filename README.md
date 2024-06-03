# Sunshine-Toolbox

Usefull commands for sunshine automatisation.
## Download

You can find an exe in the release section.<br/>
You can also use the script directly as it with python, or build your own exe with pyinstaller.

## Dependencies

Not required if you're using the provided .exe

- Python
- `pip install pywin32 PyGetWindow`
- (Optional) `pip install pyinstaller`

## Usage

### `--stream-status`: [true / false]

This will write True or False into a file (`%APPDATA%\sunshine-status\status.txt`).<br/>
I'm using this to know if a stream is running or not in another project.

### `--set-resolution`: WIDTH HEIGHT REFRESH_RATE
Self explaining.

Usefull for setting client resolution as do command:<br/>
`cmd /c "C:\path\to\sunshine-toolbox.exe" %SUNSHINE_CLIENT_WIDTH% %SUNSHINE_CLIENT_HEIGHT% %SUNSHINE_CLIENT_FPS%`

And to reset to default resolution as undo command:<br/>
`cmd /c "C:\path\to\sunshine-toolbox.exe" 2560 1440 144`

Be sure your monitor has support for your client resolution. If not, you can add them using external tools. (AMD / Nvidia software, custom-resolution-utility, etc)

### `--close-bigpicture`

This will just close steam bigpicture if the window exist.<br/>
Usefull to add as undo command for steam big picture app.

### `--bigpicture-dummy`

This is meant to be used as main command for steam big picture app.<br/>
This script will end when big picture window is closed.<br/>
I use this on my TV that has no option to quit app after a stream.<br/>
Just need to close bigpicture window to end the stream.


## Documentation

Use --help or -h to get documentation.

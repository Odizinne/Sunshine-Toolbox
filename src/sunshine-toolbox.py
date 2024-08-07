import os
import argparse
import pygetwindow as gw
import time
import sys
import win32api
import win32con
import pywintypes
from steam_language_reader import get_big_picture_window_title


class SunshineToolbox:
    BIG_PICTURE_KEYWORDS = get_big_picture_window_title().lower().split()

    def __init__(self, directory):
        self.directory = directory
        self.create_directory()

    def create_directory(self):
        if not os.path.exists(self.directory):
            os.makedirs(self.directory)

    def create_status_file(self):
        file_path = os.path.join(self.directory, "status.txt")
        with open(file_path, "w") as file:
            pass

    def delete_status_file(self):
        file_path = os.path.join(self.directory, "status.txt")
        if os.path.exists(file_path):
            os.remove(file_path)

    def big_picture_dummy(self):
        self.search_window()
        self.search_window(find_window=False)
        sys.exit()

    def close_big_picture(self):
        for window_title in gw.getAllTitles():
            if all(word.lower() in window_title.lower() for word in self.BIG_PICTURE_KEYWORDS):
                target_window = gw.getWindowsWithTitle(window_title)[0]
                print(f"Closed window: {target_window.title}")
                target_window.close()

    def search_window(self, find_window=True):
        while True:
            window_found = any(
                all(word.lower() in window_title.lower() for word in self.BIG_PICTURE_KEYWORDS)
                for window_title in gw.getAllTitles()
            )
            if window_found == find_window:
                break
            time.sleep(1)

    def change_resolution(self, width, height, refresh_rate=None, unsupported=False):
        devmode = pywintypes.DEVMODEType()
        devmode.PelsWidth = width
        devmode.PelsHeight = height
        if refresh_rate:
            devmode.DisplayFrequency = refresh_rate
        devmode.Fields = win32con.DM_PELSWIDTH | win32con.DM_PELSHEIGHT | win32con.DM_DISPLAYFREQUENCY
        flags = 0
        if unsupported:
            flags |= win32con.CDS_UPDATEREGISTRY
        result = win32api.ChangeDisplaySettings(devmode, flags)
        if result != 0:
            print(f"Failed to change display settings. Error code: {result}")
        else:
            print("Display settings changed successfully.")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--stream-on", action="store_true", help="Create status.txt in %APPDATA%/sunshine-status/")
    parser.add_argument("--stream-off", action="store_true", help="Delete status.txt in %APPDATA%/sunshine-status/")
    parser.add_argument(
        "--bigpicture-dummy",
        action="store_true",
        help="Dummy load to allow exiting stream session by closing big picture",
    )
    parser.add_argument(
        "--set-resolution",
        nargs=3,
        metavar=("WIDTH", "HEIGHT", "REFRESH_RATE"),
        help="Change display resolution and refresh rate.",
    )
    parser.add_argument("--close-bigpicture", action="store_true", help="Close Steam Big Picture window")

    args = parser.parse_args()

    directory = os.path.join(os.getenv("APPDATA"), "sunshine-status")
    toolbox = SunshineToolbox(directory)

    if not any(vars(args).values()):
        print("No arguments provided. Use --help or -h to view commands.")
        sys.exit()

    if args.stream_off:
        toolbox.delete_status_file()
        sys.exit()
    elif args.stream_on:
        toolbox.create_status_file()
        sys.exit()
    elif args.bigpicture_dummy:
        toolbox.big_picture_dummy()
        sys.exit()
    elif args.set_resolution:
        width, height, refresh_rate = map(int, args.set_resolution)
        if width and height and refresh_rate:
            toolbox.change_resolution(width, height, refresh_rate)
            print(f"Resolution changed to {width}x{height}@{refresh_rate}Hz.")
            sys.exit()
        else:
            print("Please provide width, height and refresh rate (ex 1920 1080 60).")
            sys.exit()
    elif args.close_bigpicture:
        toolbox.close_big_picture()
        sys.exit()


if __name__ == "__main__":
    main()

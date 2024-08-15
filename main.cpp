#include <QCoreApplication>
#include <iostream>
#include <windows.h>
#include <cstdlib>
#include <string>
#include <fstream>
#include <ShlObj.h>
#include <winuser.h>
#include <QString>
#include <QDir>
#include "steamwindowmanager.h"
#include <chrono>
#include <thread>

using namespace std;

// Function to get the path to %APPDATA%/sunshine-status/status.txt
wstring getStatusFilePath() {
    wchar_t path[MAX_PATH];
    if (SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, path) == S_OK) {
        qDebug() << wstring(path);
        return wstring(path) + L"\\sunshine-status\\status.txt";
    } else {
        wcerr << L"Failed to retrieve %APPDATA% path." << endl;
        return L"";
    }
}

// Function to change the display resolution
bool changePrimaryDisplayResolution(int width, int height, int refreshRate) {
    DEVMODE devmode = { {0} };
    devmode.dmSize = sizeof(DEVMODE);

    // EnumDisplaySettings with ENUM_CURRENT_SETTINGS to get current settings
    if (!EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devmode)) {
        cerr << "Failed to retrieve current display settings." << endl;
        return false;
    }

    // Set the desired width, height, and refresh rate
    devmode.dmPelsWidth = width;
    devmode.dmPelsHeight = height;
    devmode.dmDisplayFrequency = refreshRate;
    devmode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;

    // Attempt to change the display settings
    long result = ChangeDisplaySettings(&devmode, CDS_UPDATEREGISTRY);

    if (result == DISP_CHANGE_SUCCESSFUL) {
        cout << "Display resolution changed successfully to "
             << width << " X " << height
             << " at " << refreshRate << " Hz" << endl;
        return true;
    } else {
        cerr << "Failed to change display resolution. Error code: " << result << endl;
        return false;
    }
}

// Function to handle stream-on functionality
void streamOn() {
    wstring filePath = getStatusFilePath();
    if (filePath.empty()) return;

    // Extract the directory path from the full file path
    wstring dirPath = filePath.substr(0, filePath.find_last_of(L"\\"));

    // Check if the directory exists; if not, create it
    if (!QDir(QString::fromStdWString(dirPath)).exists()) {
        if (!QDir().mkpath(QString::fromStdWString(dirPath))) {
            wcerr << L"Failed to create directory: " << dirPath << endl;
            return;
        }
    }

    // Create and write to the file
    ofstream statusFile(filePath.c_str());
    if (statusFile) {
        statusFile << "streaming_on";
        statusFile.close();
        cout << "Streaming status set to ON." << endl;
    } else {
        wcerr << L"Failed to write to status file: " << filePath << endl;
    }
}


// Function to handle stream-off functionality
void streamOff() {
    wstring filePath = getStatusFilePath();
    if (filePath.empty()) return;

    if (DeleteFileW(filePath.c_str())) {
        wcout << L"Streaming status set to OFF." << endl;
    } else {
        wcerr << L"Failed to delete status file." << endl;
    }
}

void bigPictureDummyLoad() {
    using namespace std::chrono;

    auto waitInterval = milliseconds(500);  // Time to wait between checks
    auto timeout = seconds(30);  // Maximum time to wait for the window to appear

    auto start = steady_clock::now();
    bool found = false;

    // Check for the Big Picture window within the timeout period
    while (duration_cast<milliseconds>(steady_clock::now() - start) < timeout) {
        if (isBigPictureRunning()) {
            std::cout << "Big Picture window detected." << std::endl;
            found = true;
            break;
        }
        std::this_thread::sleep_for(waitInterval);
    }

    // If the Big Picture window was found, wait for it to close
    if (found) {
        std::cout << "Waiting for Big Picture window to close." << std::endl;
        while (isBigPictureRunning()) {
            std::this_thread::sleep_for(waitInterval);
        }
        std::cout << "Big Picture window closed." << std::endl;
    } else {
        std::cout << "Big Picture window did not appear within the timeout period." << std::endl;
    }
}

void printHelp() {
    cout << "Usage: [program_name] [OPTIONS]" << endl;
    cout << "Options:" << endl;
    cout << "  -h, --help                  Show this help message and exit" << endl;
    cout << "  --set-resolution width height refreshRate" << endl;
    cout << "                              Set the primary display resolution to the specified width, height, and refresh rate." << endl;
    cout << "  --stream-on                 Set the streaming status to ON by creating a status file (%appdata%\\sunshine-status\\status.txt)." << endl;
    cout << "  --stream-off                Set the streaming status to OFF by deleting the status file (%appdata%\\sunshine-status\\status.txt)." << endl;
    cout << "  --close-bigpicture          Close Steam Big Picture window if it's open." << endl;
    cout << "  --bigpicture-dummyload      Wait for Steam Big Picture window to appear and wait until it is closed, then exit the application." << endl;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printHelp();
        return 1;
    }

    string option = argv[1];

    if (option == "-h" || option == "--help") {
        printHelp();
        return 0;
    }

    if (option == "--set-resolution") {
        if (argc != 5) {
            cerr << "Usage: " << argv[0]
                 << " --set-resolution width height refreshRate" << endl;
            return 1;
        }

        int width = std::atoi(argv[2]);
        int height = std::atoi(argv[3]);
        int refreshRate = std::atoi(argv[4]);

        // Validate the arguments
        if (width <= 0 || height <= 0 || refreshRate <= 0) {
            cerr << "Width, height, and refreshRate must be positive integers." << endl;
            return 1;
        }

        if (changePrimaryDisplayResolution(width, height, refreshRate)) {
            cout << "Resolution change was successful." << endl;
        } else {
            cerr << "Resolution change failed." << endl;
        }
    } else if (option == "--stream-on") {
        streamOn();
    } else if (option == "--stream-off") {
        streamOff();
    } else if (option == "--close-bigpicture") {
        closeBigPicture();
    } else if (option == "--bigpicture-dummyload") {
        bigPictureDummyLoad();
    } else {
        cerr << "Invalid option. Use -h or --help for usage information." << endl;
        return 1;
    }

    return 0;
}

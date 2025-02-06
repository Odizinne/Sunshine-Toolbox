#include "steamwindowmanager.h"
#include <iostream>
#include <windows.h>
#include <fstream>
#include <ShlObj.h>
#include <chrono>
#include <thread>

using namespace std;
using namespace chrono;

void skipBigPictureIntro() {
    // Get the screen dimensions
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    cout << (screenHeight) << endl;

    // Calculate center position (scaled to 0-65535 range)
    int centerX = screenWidth / 2;
    int centerY = screenHeight / 2;

    INPUT input[3] = {};

    // Move mouse to center (scaled properly to 0-65535)
    input[0].type = INPUT_MOUSE;
    input[0].mi.dx = centerX * (65535 / screenWidth);
    input[0].mi.dy = centerY * (65535 / screenHeight);
    input[0].mi.mouseData = 0;
    input[0].mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
    input[0].mi.time = 0;
    input[0].mi.dwExtraInfo = 0;

    input[1].type = INPUT_MOUSE;
    input[1].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

    input[2].type = INPUT_MOUSE;
    input[2].mi.dwFlags = MOUSEEVENTF_LEFTUP;

    SendInput(3, input, sizeof(INPUT));
}

// Function to get the path to %APPDATA%/sunshine-status/status.txt
wstring getStatusFilePath() {
    wchar_t path[MAX_PATH];
    if (SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, path) == S_OK) {
        return wstring(path) + L"\\sunshine-status\\status.txt";
    } else {
        wcerr << L"Failed to retrieve %APPDATA% path." << endl;
        return L"";
    }
}

void streamOn() {
    wstring filePath = getStatusFilePath();
    if (filePath.empty()) return;

    // Extract the directory path from the full file path
    wstring dirPath = filePath.substr(0, filePath.find_last_of(L"\\"));

    // Check if the directory exists; if not, create it
    DWORD fileAttributes = GetFileAttributesW(dirPath.c_str());
    if (fileAttributes == INVALID_FILE_ATTRIBUTES) {
        if (!CreateDirectoryW(dirPath.c_str(), nullptr)) {
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

void runBigPicture(bool skipIntro) {
    // Retrieve SteamPath from the registry
    HKEY hKey;
    char steamPath[MAX_PATH];
    DWORD bufferSize = sizeof(steamPath);

    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Valve\\Steam", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        if (RegQueryValueExA(hKey, "SteamPath", nullptr, nullptr, reinterpret_cast<LPBYTE>(steamPath), &bufferSize) == ERROR_SUCCESS) {
            RegCloseKey(hKey);

            // Append the Steam executable and gamepadui flag
            string command = string(steamPath) + "\\steam.exe steam://open/bigpicture";

            // Launch Steam in detached mode
            STARTUPINFOA si = { sizeof(STARTUPINFOA) };
            PROCESS_INFORMATION pi;
            if (CreateProcessA(nullptr, const_cast<char*>(command.c_str()), nullptr, nullptr, FALSE, DETACHED_PROCESS, nullptr, nullptr, &si, &pi)) {
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
                cout << "Launching steam in gamepadui mode." << endl;
            } else {
                cerr << "Failed to launch Steam in gamepadui mode." << endl;
                return;
            }
        } else {
            RegCloseKey(hKey);
            cerr << "Failed to retrieve SteamPath from the registry." << endl;
            return;
        }
    } else {
        cerr << "Failed to open Steam registry key." << endl;
        return;
    }

    chrono::milliseconds waitInterval(500);  // Time to wait between checks
    chrono::seconds timeout(30);  // Maximum time to wait for the window to appear

    chrono::steady_clock::time_point start = chrono::steady_clock::now();
    bool found = false;

    // Check for the Big Picture window within the timeout period
    while (duration_cast<milliseconds>(steady_clock::now() - start) < timeout) {
        if (isBigPictureRunning()) {
            cout << "Big Picture window detected." << endl;
            found = true;
            break;
        }
        this_thread::sleep_for(waitInterval);
    }

    if (skipIntro) {
        skipBigPictureIntro();
        this_thread::sleep_for(waitInterval * 4); // sleep here to prevent crash if skipping intro
    }

    if (found) {
        cout << "Waiting for Big Picture window to close." << endl;
        while (isBigPictureRunning()) {
            this_thread::sleep_for(waitInterval);
        }
        cout << "Big Picture window closed." << endl;
    } else {
        cout << "Big Picture window did not appear within the timeout period." << endl;
    }
}

void shutdownHost() {
    system("shutdown /s /f /t 0");
}

void printHelp() {
    cout << "Usage: [sunshine-toolbox.exe] [OPTIONS]" << endl;
    cout << "Options:" << endl;
    cout << "  -h, --help                       Show this help message and exit" << endl;
    cout << "  --stream-on || --stream-off      Scripting purpose: create or delete a status file (%appdata%\\sunshine-status\\status.txt)." << endl;
    cout << "  --close-bigpicture               Close Steam Big Picture window if it's open." << endl;
    cout << "  --run-bigpicture                 Start steam in BigPicture mode and wait until it is closed, then exit the application." << endl;
    cout << "  --shutdown                       Shutdown host PC." << endl;
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

    if (option == "--stream-on") {
        streamOn();
    } else if (option == "--stream-off") {
        streamOff();
    } else if (option == "--close-bigpicture") {
        closeBigPicture();
    } else if (option == "--run-bigpicture") {
        // Check if there is an extra argument for --skip-intro
        bool skipIntro = false;
        for (int i = 2; i < argc; ++i) {
            if (string(argv[i]) == "--skip-intro") {
                skipIntro = true;
                break;
            }
        }
        runBigPicture(skipIntro);
    } else if (option == "--shutdown") {
        shutdownHost();
    } else {
        cerr << "Invalid option. Use -h or --help for usage information." << endl;
        return 1;
    }

    return 0;
}

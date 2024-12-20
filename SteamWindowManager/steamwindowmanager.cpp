#include "steamwindowmanager.h"
#include <algorithm>
#include <iostream>
#include <map>
#include <sstream>
#include <codecvt>
#include <locale>
#include <Windows.h>
#include <vector>
#include <winreg.h>

// Define the map
const std::map<std::string, std::string> BIG_PICTURE_WINDOW_TITLES = {
    {"schinese", "Steam 大屏幕模式"},
    {"tchinese", "Steam Big Picture 模式"},
    {"japanese", "Steam Big Pictureモード"},
    {"koreana", "Steam Big Picture 모드"},
    {"thai", "โหมด Big Picture บน Steam"},
    {"bulgarian", "Steam режим „Голям екран“"},
    {"czech", "Steam režim Big Picture"},
    {"danish", "Steam Big Picture-tilstand"},
    {"german", "Big-Picture-Modus"},
    {"english", "Steam Big Picture mode"},
    {"spanish", "Modo Big Picture de Steam"},
    {"latam", "Modo Big Picture de Steam"},
    {"greek", "Steam Λειτουργία Big Picture"},
    {"french", "Steam mode Big Picture"},
    {"indonesian", "Mode Big Picture Steam"},
    {"italian", "Modalità Big Picture di Steam"},
    {"hungarian", "Steam Nagy Kép mód"},
    {"dutch", "Steam Big Picture-modus"},
    {"norwegian", "Steam Big Picture-modus"},
    {"polish", "Tryb Big Picture Steam"},
    {"portuguese", "Steam Big Picture"},
    {"brazilian", "Steam Modo Big Picture"},
    {"romanian", "Steam modul Big Picture"},
    {"russian", "Режим Big Picture"},
    {"finnish", "Steamin televisiotila"},
    {"swedish", "Steams Big Picture-läge"},
    {"turkish", "Steam Geniş Ekran Modu"},
    {"vietnamese", "Chế độ Big Picture trên Steam"},
    {"ukrainian", "Steam у режимі Big Picture"}
};

std::string getRegistryValue(const std::wstring& keyPath, const std::wstring& valueName) {
    HKEY hKey;
    char value[256];
    DWORD valueLength = sizeof(value);
    std::string result;

    // Using A-version of Reg functions
    if (RegOpenKeyExA(HKEY_CURRENT_USER, std::string(keyPath.begin(), keyPath.end()).c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        if (RegQueryValueExA(hKey, std::string(valueName.begin(), valueName.end()).c_str(), NULL, NULL, (LPBYTE)value, &valueLength) == ERROR_SUCCESS) {
            result = value;
        }
        RegCloseKey(hKey);
    }

    return result;
}

std::string getSteamLanguage() {
    return getRegistryValue(L"Software\\Valve\\Steam\\steamglobal", L"Language");
}

std::string cleanString(const std::string& str) {
    std::string cleanedStr = str;
    cleanedStr.erase(std::remove(cleanedStr.begin(), cleanedStr.end(), '\xA0'), cleanedStr.end()); // Remove non-breaking space
    return cleanedStr;
}

std::string getBigPictureWindowTitle() {
    std::string language = getSteamLanguage();
    std::transform(language.begin(), language.end(), language.begin(), ::tolower);

    auto it = BIG_PICTURE_WINDOW_TITLES.find(language);
    if (it != BIG_PICTURE_WINDOW_TITLES.end()) {
        return it->second;
    }
    return BIG_PICTURE_WINDOW_TITLES.at("english");
}

std::vector<std::string> getAllWindowTitles() {
    std::vector<std::string> windowTitles;

    EnumWindows(
        [](HWND hwnd, LPARAM lParam) -> BOOL {
            std::vector<std::string>* titles = reinterpret_cast<std::vector<std::string>*>(lParam);

            if (IsWindowVisible(hwnd) && !(GetWindowLong(hwnd, GWL_STYLE) & WS_MINIMIZE)) {
                WCHAR windowTitle[256];
                if (GetWindowTextW(hwnd, windowTitle, sizeof(windowTitle) / sizeof(WCHAR)) > 0) {
                    std::wstring ws(windowTitle);
                    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
                    titles->push_back(converter.to_bytes(ws));
                }
            }
            return TRUE;
        },
        reinterpret_cast<LPARAM>(&windowTitles));

    return windowTitles;
}

bool isBigPictureRunning() {
    std::string bigPictureTitle = cleanString(getBigPictureWindowTitle());
    std::istringstream iss(bigPictureTitle);
    std::vector<std::string> bigPictureWords((std::istream_iterator<std::string>(iss)), std::istream_iterator<std::string>());

    auto currentWindowTitles = getAllWindowTitles();
    for (const auto& windowTitle : currentWindowTitles) {
        std::string cleanedTitle = cleanString(windowTitle);
        std::istringstream issWindow(cleanedTitle);
        std::vector<std::string> windowWords((std::istream_iterator<std::string>(issWindow)), std::istream_iterator<std::string>());

        if (std::all_of(bigPictureWords.begin(), bigPictureWords.end(), [&windowWords](const std::string& word) {
                return std::find(windowWords.begin(), windowWords.end(), word) != windowWords.end();
            })) {
            return true;
        }
    }
    return false;
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    struct EnumWindowsData {
        std::vector<std::string> expectedWords;
        bool windowFound;
    };
    EnumWindowsData* data = reinterpret_cast<EnumWindowsData*>(lParam);

    wchar_t windowTitle[256];
    if (IsWindowVisible(hwnd) && GetWindowTextW(hwnd, windowTitle, sizeof(windowTitle) / sizeof(wchar_t)) > 0) {
        std::wstring ws(windowTitle);
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        std::string title = cleanString(converter.to_bytes(ws));

        std::istringstream iss(title);
        std::vector<std::string> titleWords((std::istream_iterator<std::string>(iss)), std::istream_iterator<std::string>());

        bool match = true;
        for (const auto& word : data->expectedWords) {
            if (std::find(titleWords.begin(), titleWords.end(), word) == titleWords.end()) {
                match = false;
                break;
            }
        }

        if (match) {
            PostMessage(hwnd, WM_CLOSE, 0, 0);
            std::cout << "Closed Big Picture window: " << title << std::endl;

            data->windowFound = true;
            return FALSE;
        }
    }
    return TRUE;
}

void closeBigPicture() {
    std::string bigPictureTitle = cleanString(getBigPictureWindowTitle());
    std::istringstream iss(bigPictureTitle);
    std::vector<std::string> expectedWords((std::istream_iterator<std::string>(iss)), std::istream_iterator<std::string>());

    std::cout << "Looking for Big Picture window with title words: ";
    for (const auto& word : expectedWords) {
        std::cout << word << " ";
    }
    std::cout << std::endl;

    struct EnumWindowsData {
        std::vector<std::string> expectedWords;
        bool windowFound;
    };
    EnumWindowsData data = {expectedWords, false};

    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&data));

    if (!data.windowFound) {
        std::cout << "No matching Big Picture window found." << std::endl;
    }
}

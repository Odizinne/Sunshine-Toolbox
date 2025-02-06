#include "steamwindowmanager.h"
#include <windows.h>
#include <winreg.h>
#include <iostream>
#include <unordered_map>
#include <algorithm>

// Define the map
const std::unordered_map<std::string, std::string> BIG_PICTURE_WINDOW_TITLES
    = {{"schinese", "Steam 大屏幕模式"},
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

std::string getRegistryValue(const std::wstring &keyPath, const std::wstring &valueName)
{
    HKEY hKey;
    WCHAR value[256];
    DWORD valueLength = sizeof(value);
    std::string result;

    if (RegOpenKeyExW(HKEY_CURRENT_USER, keyPath.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        if (RegQueryValueExW(hKey, valueName.c_str(), NULL, NULL, (LPBYTE)value, &valueLength)
            == ERROR_SUCCESS) {
            std::wstring wstr(value);
            result.resize(wstr.length());
            std::transform(wstr.begin(), wstr.end(), result.begin(), ::tolower);
        }
        RegCloseKey(hKey);
    }

    return result;
}

std::string getSteamLanguage()
{
    return getRegistryValue(L"Software\\Valve\\Steam\\steamglobal", L"Language");
}

std::string cleanString(const std::string &str)
{
    std::string cleanedStr = str;
    size_t pos = 0;
    while ((pos = cleanedStr.find('\xA0')) != std::string::npos) {
        cleanedStr.replace(pos, 1, " ");
    }
    return cleanedStr;
}

std::string getBigPictureWindowTitle()
{
    std::string language = getSteamLanguage();
    auto it = BIG_PICTURE_WINDOW_TITLES.find(language);
    if (it == BIG_PICTURE_WINDOW_TITLES.end()) {
        it = BIG_PICTURE_WINDOW_TITLES.find("english");
    }
    return it->second;
}

std::vector<std::string> getAllWindowTitles()
{
    std::vector<std::string> windowTitles;

    EnumWindows(
        [](HWND hwnd, LPARAM lParam) -> BOOL {
            auto *titles = reinterpret_cast<std::vector<std::string>*>(lParam);

            if (IsWindowVisible(hwnd) && !(GetWindowLong(hwnd, GWL_STYLE) & WS_MINIMIZE)) {
                WCHAR windowTitle[256];
                if (GetWindowTextW(hwnd, windowTitle, sizeof(windowTitle) / sizeof(WCHAR)) > 0) {
                    std::wstring wstr(windowTitle);
                    titles->push_back(std::string(wstr.begin(), wstr.end()));
                }
            }
            return TRUE;
        },
        reinterpret_cast<LPARAM>(&windowTitles));

    return windowTitles;
}

bool isBigPictureRunning()
{
    std::string bigPictureTitle = cleanString(getBigPictureWindowTitle());
    std::transform(bigPictureTitle.begin(), bigPictureTitle.end(), bigPictureTitle.begin(), ::tolower);

    std::vector<std::string> bigPictureWords;
    std::string word;
    for (char c : bigPictureTitle) {
        if (c == ' ') {
            if (!word.empty()) {
                bigPictureWords.push_back(word);
                word.clear();
            }
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        bigPictureWords.push_back(word);
    }

    std::vector<std::string> currentWindowTitles = getAllWindowTitles();
    for (const auto &windowTitle : currentWindowTitles) {
        std::string cleanedTitle = cleanString(windowTitle);
        std::transform(cleanedTitle.begin(), cleanedTitle.end(), cleanedTitle.begin(), ::tolower);

        std::vector<std::string> windowWords;
        word.clear();
        for (char c : cleanedTitle) {
            if (c == ' ') {
                if (!word.empty()) {
                    windowWords.push_back(word);
                    word.clear();
                }
            } else {
                word += c;
            }
        }
        if (!word.empty()) {
            windowWords.push_back(word);
        }

        bool found = true;
        for (const auto &bpWord : bigPictureWords) {
            if (std::find(windowWords.begin(), windowWords.end(), bpWord) == windowWords.end()) {
                found = false;
                break;
            }
        }

        if (found) {
            return true;
        }
    }
    return false;
}

struct EnumWindowsData {
    std::vector<std::string> expectedWords;
    bool windowFound;
};

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    auto* data = reinterpret_cast<EnumWindowsData*>(lParam);
    wchar_t windowTitle[256];
    if (IsWindowVisible(hwnd) && GetWindowTextW(hwnd, windowTitle, sizeof(windowTitle) / sizeof(wchar_t)) > 0) {
        std::wstring wstrTitle(windowTitle);
        std::string title(wstrTitle.begin(), wstrTitle.end());
        std::transform(title.begin(), title.end(), title.begin(), ::tolower);

        size_t pos = 0;
        while ((pos = title.find('\xA0')) != std::string::npos) {
            title.replace(pos, 1, " ");
        }

        std::vector<std::string> titleWords;
        std::string word;
        for (char c : title) {
            if (c == ' ') {
                if (!word.empty()) {
                    titleWords.push_back(word);
                    word.clear();
                }
            } else {
                word += c;
            }
        }
        if (!word.empty()) {
            titleWords.push_back(word);
        }

        bool match = true;
        for (const std::string& expectedWord : data->expectedWords) {
            if (std::find(titleWords.begin(), titleWords.end(), expectedWord) == titleWords.end()) {
                match = false;
                break;
            }
        }

        if (match) {
            PostMessage(hwnd, WM_CLOSE, 0, 0);
            std::wcout << L"Closed Big Picture window: " << wstrTitle << std::endl;

            data->windowFound = true;
            return FALSE;
        }
    }
    return TRUE;
}

void closeBigPicture() {
    std::string bigPictureTitle = getBigPictureWindowTitle();
    std::transform(bigPictureTitle.begin(), bigPictureTitle.end(), bigPictureTitle.begin(), ::tolower);
    size_t pos = 0;
    while ((pos = bigPictureTitle.find('\xA0')) != std::string::npos) {
        bigPictureTitle.replace(pos, 1, " ");
    }

    std::vector<std::string> expectedWords;
    std::string word;
    for (char c : bigPictureTitle) {
        if (c == ' ') {
            if (!word.empty()) {
                expectedWords.push_back(word);
                word.clear();
            }
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        expectedWords.push_back(word);
    }

    std::wcout << L"Looking for Big Picture window with title words: ";
    for (const auto& w : expectedWords) {
        std::wcout << std::wstring(w.begin(), w.end()) << L" ";
    }
    std::wcout << std::endl;

    EnumWindowsData data = { expectedWords, false };

    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&data));

    if (!data.windowFound) {
        std::wcout << L"No matching Big Picture window found." << std::endl;
    }
}

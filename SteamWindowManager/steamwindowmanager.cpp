#include "steamwindowmanager.h"
#include <QMap>
#include <QStringList>
#include <QVector>
#include <windows.h>
#include <winreg.h>
#include <iostream>

// Define the map
const QMap<QString, QString> BIG_PICTURE_WINDOW_TITLES
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
       {"ukrainian", "Steam у режимі Big Picture"}};

const QChar NON_BREAKING_SPACE = QChar(0x00A0);

QString getRegistryValue(const std::wstring &keyPath, const std::wstring &valueName)
{
    HKEY hKey;
    WCHAR value[256];
    DWORD valueLength = sizeof(value);
    QString result;

    if (RegOpenKeyEx(HKEY_CURRENT_USER, keyPath.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        if (RegQueryValueEx(hKey, valueName.c_str(), NULL, NULL, (LPBYTE) value, &valueLength)
            == ERROR_SUCCESS) {
            result = QString::fromWCharArray(value).toLower();
        }
        RegCloseKey(hKey);
    }

    return result;
}

QString getSteamLanguage()
{
    return getRegistryValue(L"Software\\Valve\\Steam\\steamglobal", L"Language");
}

QString cleanString(const QString &str)
{
    QString cleanedStr = str;
    return cleanedStr.replace(NON_BREAKING_SPACE, ' ');
}

QString getBigPictureWindowTitle()
{
    QString language = getSteamLanguage().toLower();

    return BIG_PICTURE_WINDOW_TITLES.value(language, BIG_PICTURE_WINDOW_TITLES.value("english"));
}

QVector<QString> getAllWindowTitles()
{
    QVector<QString> windowTitles;

    EnumWindows(
        [](HWND hwnd, LPARAM lParam) -> BOOL {
            QVector<QString> *titles = reinterpret_cast<QVector<QString> *>(lParam);

            if (IsWindowVisible(hwnd) && !(GetWindowLong(hwnd, GWL_STYLE) & WS_MINIMIZE)) {
                WCHAR windowTitle[256];
                if (GetWindowText(hwnd, windowTitle, sizeof(windowTitle) / sizeof(WCHAR)) > 0) {
                    titles->append(QString::fromWCharArray(windowTitle));
                }
            }
            return TRUE;
        },
        reinterpret_cast<LPARAM>(&windowTitles));

    return windowTitles;
}

bool isBigPictureRunning()
{
    QString bigPictureTitle = cleanString(getBigPictureWindowTitle().toLower());
    QStringList bigPictureWords = bigPictureTitle.split(' ', Qt::SkipEmptyParts);

    QVector<QString> currentWindowTitles = getAllWindowTitles();
    for (const auto &windowTitle : currentWindowTitles) {
        QString cleanedTitle = cleanString(windowTitle.toLower());
        QStringList windowWords = cleanedTitle.split(' ', Qt::SkipEmptyParts);

        if (std::all_of(bigPictureWords.begin(),
                        bigPictureWords.end(),
                        [&windowWords](const QString &word) {
                            return windowWords.contains(word);
                        })) {
            return true;
        }
    }
    return false;
}

using namespace std;

struct EnumWindowsData {
    QStringList expectedWords;
    bool windowFound;
};

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    EnumWindowsData* data = reinterpret_cast<EnumWindowsData*>(lParam);

    wchar_t windowTitle[256];
    if (IsWindowVisible(hwnd) && GetWindowText(hwnd, windowTitle, sizeof(windowTitle) / sizeof(wchar_t)) > 0) {
        QString title = QString::fromWCharArray(windowTitle).toLower().replace(NON_BREAKING_SPACE, ' ');

        QStringList titleWords = title.split(' ', Qt::SkipEmptyParts);

        bool match = true;
        for (const QString& word : data->expectedWords) {
            if (!titleWords.contains(word)) {
                match = false;
                break;
            }
        }

        if (match) {
            PostMessage(hwnd, WM_CLOSE, 0, 0);
            wcout << L"Closed Big Picture window: " << title.toStdWString() << endl;

            data->windowFound = true;
            return FALSE;
        }
    }
    return TRUE;
}

void closeBigPicture() {
    QString bigPictureTitle = getBigPictureWindowTitle().toLower().replace(NON_BREAKING_SPACE, ' ');
    QStringList expectedWords = bigPictureTitle.split(' ', Qt::SkipEmptyParts);
    wcout << L"Looking for Big Picture window with title words: " << expectedWords.join(" ").toStdWString() << endl;

    EnumWindowsData data = { expectedWords, false };

    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&data));

    if (!data.windowFound) {
        wcout << L"No matching Big Picture window found." << endl;
    }
}

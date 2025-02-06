QT += core

CONFIG += c++17 cmdline silent

INCLUDEPATH += \
        SteamWindowManager \

HEADERS += \
        SteamWindowManager/steamwindowmanager.h \

SOURCES += \
        SteamWindowManager/steamwindowmanager.cpp \
        main.cpp

LIBS += -lUser32 -lShell32 -lAdvapi32


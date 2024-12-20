CONFIG += c++17 cmdline silent

INCLUDEPATH += \
        SteamWindowManager \
        Dependencies/HDRTray/common

HEADERS += \
        SteamWindowManager/steamwindowmanager.h \
        Dependencies/HDRTray/common/hdr.h \

SOURCES += \
        SteamWindowManager/steamwindowmanager.cpp \
        Dependencies/HDRTray/common/hdr.cpp \
        main.cpp

LIBS += -lUser32 -lShell32 -lAdvapi32


QT = core

CONFIG += c++17 cmdline

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

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

LIBS += -lUser32.lib -lShell32 -lAdvapi32

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

QT = core

CONFIG += c++17 cmdline

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += \
        SteamWindowManager/

HEADERS += \
        SteamWindowManager/steamwindowmanager.h

SOURCES += \
        SteamWindowManager/steamwindowmanager.cpp \
        main.cpp

LIBS += user32.lib \
        Shell32.lib \
        advapi32.lib

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

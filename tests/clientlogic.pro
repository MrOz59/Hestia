QT += core qml testlib

TEMPLATE = app
TARGET = tst_clientlogic

CONFIG += console testcase c++17 link_pkgconfig
PKGCONFIG += sdl2

INCLUDEPATH += \
    ../app \
    ../moonlight-common-c/moonlight-common-c/src

SOURCES += \
    tst_clientlogic.cpp \
    ../app/backend/hestiacapabilities.cpp \
    ../app/settings/presetconfiguration.cpp \
    ../app/streaming/hestianegotiation.cpp \
    ../app/streaming/video/statsdiagnostics.cpp

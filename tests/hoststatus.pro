QT += core testlib

TEMPLATE = app
TARGET = tst_hestiahoststatus
CONFIG += console testcase c++17

INCLUDEPATH += ../app

SOURCES += \
    tst_hestiahoststatus.cpp \
    ../app/backend/hestiacapabilities.cpp \
    ../app/backend/hestiahoststatus.cpp

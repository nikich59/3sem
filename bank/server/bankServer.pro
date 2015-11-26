TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.c

QMAKE_CFLAGS += -std=c11
QMAKE_CFLAGS += -pthread
QMAKE_LFLAGS += -pthread

HEADERS += \
    server.h


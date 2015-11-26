TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.c

QMAKE_CFLAGS += -lpthread

HEADERS += \
    client.h


TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS += -std=c++0x -pthread
LIBS += -pthread

SOURCES += \
    server.c

DISTFILES += \
    mathserver.pro.user \
    temp.txt

HEADERS += \
    mathserver.h \
    client.h


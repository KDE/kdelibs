TEMPLATE = lib
SOURCES += WeaverThreadGrid.cpp
HEADERS += WeaverThreadGrid.h
CONFIG += dll warn_on thread
VERSION = 0.6.0
INCLUDEPATH += ../Weaver/
LIBS += -L../Weaver -lThreadWeaver
TARGET = ThreadWeaverGui

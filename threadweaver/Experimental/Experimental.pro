TEMPLATE = lib
SOURCES += FileLoaderJob.cpp
HEADERS += FileLoaderJob.h
CONFIG += dll warn_on thread
VERSION = 0.4.0
INCLUDEPATH += ../Weaver/
LIBS += -L../Weaver -lThreadWeaver
TARGET = ThreadWeaverExperimental

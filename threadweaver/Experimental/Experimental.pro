TEMPLATE = lib
SOURCES += FailableJob.cpp FileLoaderJob.cpp
HEADERS += FailableJob.h FileLoaderJob.h
CONFIG += dll warn_on debug thread
VERSION = 0.4.0
INCLUDEPATH += ../Weaver/
TARGET = ThreadWeaverExperimental

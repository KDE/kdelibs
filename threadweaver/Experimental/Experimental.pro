TEMPLATE = lib
SOURCES += FailableJob.cpp FileLoaderJob.cpp JobSequence.cpp
HEADERS += FailableJob.h FileLoaderJob.h JobSequence.h
CONFIG += dll warn_on debug thread
VERSION = 0.4.0
INCLUDEPATH += ../Weaver/
TARGET = ThreadWeaverExperimental

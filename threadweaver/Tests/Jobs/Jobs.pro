TEMPLATE = app
SOURCES += Jobs.cpp WeaverObserverTest.cpp
HEADERS += Jobs.h WeaverObserverTest.h
FORMS += JobsBase.ui
INCLUDEPATH += ../../Weaver/
LIBS += -L../../Weaver -lThreadWeaver
CONFIG += qt warn_on debug thread
TARGET = Jobs

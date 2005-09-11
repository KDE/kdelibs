TEMPLATE = app
SOURCES += Jobs.cpp WeaverObserverTest.cpp WeaverThreadGrid.cpp
HEADERS += Jobs.h WeaverObserverTest.h WeaverThreadGrid.h
FORMS += JobsBase.ui
INCLUDEPATH += ../../Weaver/
LIBS += -L../../Weaver -lThreadWeaver
CONFIG += qt warn_on debug thread
TARGET = Jobs

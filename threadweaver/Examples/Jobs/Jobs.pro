TEMPLATE = app
SOURCES += Jobs.cpp WeaverObserverTest.cpp WeaverThreadGrid.cpp
HEADERS += Jobs.h WeaverObserverTest.h WeaverThreadGrid.h
FORMS += JobsBase.ui
INCLUDEPATH += ../../Weaver/ ../../Experimental
LIBS += -L../../Weaver -lThreadWeaver -L../../Experimental -lThreadWeaverExperimental
CONFIG += qt warn_on thread
TARGET = Jobs

TEMPLATE = app
SOURCES += Jobs.cpp WeaverObserverTest.cpp
HEADERS += Jobs.h WeaverObserverTest.h
FORMS += JobsBase.ui
INCLUDEPATH += ../../Weaver/ ../../WeaverGui
LIBS += -L../../Weaver -lThreadWeaver -L../../WeaverGui -lThreadWeaverGui
CONFIG += qt warn_on thread
TARGET = Jobs

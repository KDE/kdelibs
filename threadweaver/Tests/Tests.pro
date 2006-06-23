TEMPLATE = app
TARGET = JobTests
DEPENDPATH += .
INCLUDEPATH += ../Weaver ../Experimental 
LIBS += -L../Weaver -lThreadWeaver -L../Experimental -lThreadWeaverExperimental
CONFIG += qtestlib

test.target = test
test.commands = LD_LIBRARY_PATH=../Weaver:../Experimental ./$(TARGET)
test.depends = $(TARGET)
QMAKE_EXTRA_TARGETS += test

SOURCES += JobTests.cpp

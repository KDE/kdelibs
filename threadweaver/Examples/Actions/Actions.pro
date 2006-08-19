TEMPLATE = app
INCLUDEPATH += . ../../Weaver/ ../../Action/
LIBS += -L../../Weaver -lThreadWeaver -L../../Action -lAction
CONFIG += qt warn_on thread

# Input
HEADERS += Example.h
FORMS += Example.ui
SOURCES += Example.cpp \
           main.cpp

TEMPLATE = lib
INCLUDEPATH += ../Weaver/
LIBS += -L../Weaver -lThreadWeaver
CONFIG += qt warn_on thread
QT -= gui

HEADERS += Action.h \
    ExecutePolicy.h \
    ActionJob_p.h \
    JobsListPolicy.h
SOURCES += Action.cpp \
    ExecutePolicy.cpp \
    ActionJob_p.cpp \
    JobsListPolicy.cpp

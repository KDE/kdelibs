TEMPLATE = app
SOURCES += main.cpp QProcessJob.cpp
HEADERS += QProcessJob.h
# FORMS += JobsBase.ui
INCLUDEPATH += ../../Weaver/
LIBS += -L../../Weaver -lThreadWeaver
CONFIG += qt warn_on debug thread
TARGET = QProcessExample

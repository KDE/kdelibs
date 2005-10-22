TEMPLATE = app
SOURCES += main.cpp QProcessJob.cpp MainWindow.cpp
HEADERS += QProcessJob.h MainWindow.h
FORMS += MainWindowUi.ui

INCLUDEPATH += ../../Weaver/

LIBS += -L../../Weaver -lThreadWeaver

CONFIG += qt warn_on debug thread

TARGET = QProcessJob

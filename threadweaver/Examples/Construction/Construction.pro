TEMPLATE = app
SOURCES += Construction.cpp
INCLUDEPATH += ../../Weaver/ 
LIBS += -L../../Weaver -lThreadWeaver
CONFIG += qt warn_on debug thread
TARGET = Construction


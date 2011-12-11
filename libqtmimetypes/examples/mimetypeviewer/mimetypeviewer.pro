#-------------------------------------------------
#
# Project created by QtCreator 2011-05-07T14:29:03
#
#-------------------------------------------------

QT       += core gui

QT += widgets

TARGET = MimeTypeViewer
TEMPLATE = app

message($$PWD)

LIBS += -L$$OUT_PWD/../../src/mimetypes -lQtMimeTypes

INCLUDEPATH *= $$PWD/../../include/QtMimeTypes
CONFIG += depend_includepath

SOURCES += main.cpp\
    mimetypeviewer.cpp

HEADERS  += mimetypeviewer.h

FORMS    += mimetypeviewer.ui

QMAKE_CXXFLAGS += -W -Wall -Wextra -Werror -Wshadow -Wno-long-long -Wnon-virtual-dtor
mac|darwin: {
    QMAKE_CXXFLAGS += -ansi
} else:false {
    QMAKE_CXXFLAGS += -ansi -Wc++0x-compat
} else {
    QMAKE_CXXFLAGS += -std=c++0x
}

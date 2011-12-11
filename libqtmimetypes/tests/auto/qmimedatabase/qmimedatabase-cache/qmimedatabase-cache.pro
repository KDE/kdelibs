QT       += testlib

QT       -= widgets gui

TARGET = tst_qmimedatabase-cache
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

LIBS += -L$$OUT_PWD/../../../../src/mimetypes -lQtMimeTypes

INCLUDEPATH *= $$PWD/../../../../include/QtMimeTypes
INCLUDEPATH *= $$PWD/../../../../src/mimetypes/inqt5
CONFIG += depend_includepath

SOURCES += tst_qmimedatabase-cache.cpp
HEADERS += ../tst_qmimedatabase.h
DEFINES += SRCDIR='"\\"$$PWD/../\\""'

QMAKE_CXXFLAGS += -W -Wall -Wextra -Werror -Wshadow -Wno-long-long -Wnon-virtual-dtor
mac|darwin: {
    QMAKE_CXXFLAGS += -ansi
} else:false {
    QMAKE_CXXFLAGS += -ansi -Wc++0x-compat
} else {
    QMAKE_CXXFLAGS += -std=c++0x
}

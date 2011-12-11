QT       += core

TEMPLATE = app

LIBS += -L$$OUT_PWD/../../src/mimetypes -lQtMimeTypes

INCLUDEPATH *= $$PWD/../../include/QtMimeTypes
CONFIG += depend_includepath

SOURCES += main.cpp

QMAKE_CXXFLAGS += -W -Wall -Wextra -Werror -Wshadow -Wno-long-long -Wnon-virtual-dtor
mac|darwin: {
    QMAKE_CXXFLAGS += -ansi
} else:false {
    QMAKE_CXXFLAGS += -ansi -Wc++0x-compat
} else {
    QMAKE_CXXFLAGS += -std=c++0x
}

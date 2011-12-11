CONFIG   += qtestlib
TEMPLATE = app
TARGET   = tst_qmimetype
DEPENDPATH += .

QMAKE_CXXFLAGS += -W -Wall -Wextra -Werror -Wshadow -Wno-long-long -Wnon-virtual-dtor
mac|darwin: {
    QMAKE_CXXFLAGS += -ansi
} else:false {
    QMAKE_CXXFLAGS += -ansi -Wc++0x-compat
} else {
    QMAKE_CXXFLAGS += -std=c++0x
}

# runtime environment
LIBS += -L../../../src/mimetypes -lQtMimeTypes


API_DIR = ..

INCLUDEPATH += ../../../include/QtMimeTypes ../../../src/mimetypes $$API_DIR/unittests
CONFIG += depend_includepath


SOURCES += tst_qmimetype.cpp

HEADERS += tst_qmimetype.h


QMAKE_EXTRA_TARGETS += check
check.depends = $$TARGET
check.commands = ./$$TARGET -xunitxml -o unitTestResults.xml

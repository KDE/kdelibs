TARGET = QtMimeTypes
TEMPLATE = lib

win32: DESTDIR = ./

DEFINES += QMIME_LIBRARY

INCLUDEPATH *= $$PWD/../../include/QtMimeTypes
DEPENDPATH  *= $$PWD

INCLUDEPATH += $$PWD/inqt5
CONFIG += depend_includepath

QT     = core

DEFINES += QT_NO_CAST_FROM_ASCII

QMAKE_CXXFLAGS += -W -Wall -Wextra -Werror -Wshadow -Wno-long-long -Wnon-virtual-dtor
mac|darwin: {
    QMAKE_CXXFLAGS += -ansi
} else:false {
    QMAKE_CXXFLAGS += -ansi -Wc++0x-compat
} else {
    QMAKE_CXXFLAGS += -std=c++0x
}

SOURCES += qmimedatabase.cpp \
    qmimetype.cpp \
    qmimemagicrulematcher.cpp \
    mimetypeparser.cpp \
    qmimemagicrule.cpp \
    qmimeglobpattern.cpp \
    qmimeprovider.cpp

HEADERS += qmime_global.h \
    qmimedatabase.h \
    qmimetype.h \
    qmimemagicrulematcher_p.h \
    qmimetype_p.h \
    mimetypeparser_p.h \
    qmimedatabase_p.h \
    qmimemagicrule_p.h \
    qmimeglobpattern_p.h \
    qmimeprovider_p.h

SOURCES += inqt5/qstandardpaths.cpp
win32: SOURCES += inqt5/qstandardpaths_win.cpp
unix: {
    macx-*: {
        SOURCES += inqt5/qstandardpaths_mac.cpp
    } else {
        SOURCES += inqt5/qstandardpaths_unix.cpp
    }
}

RESOURCES += \
    mimetypes.qrc

symbian {
    MMP_RULES += EXPORTUNFROZEN
    TARGET.UID3 = 0xEA6A790B
    TARGET.CAPABILITY =
    TARGET.EPOCALLOWDLLDATA = 1
    addFiles.sources = lib.dll
    addFiles.path = !:/sys/bin
    DEPLOYMENT += addFiles
}

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

CONFIG   += qt plugin
TEMPLATE = lib
TARGET   = declarative_mimetypes

# QtCore/qlist.h uses /usr/include/limits.h which uses does not compile with -pedantic.
# QtDeclarative/qdeclarativeprivate.h will not compile with -pedantic.
#MAKE_CXXFLAGS += -W -Wall -Wextra -Werror -pedantic -Wshadow -Wno-long-long -Wnon-virtual-dtor
QMAKE_CXXFLAGS += -W -Wall -Wextra -Werror           -Wshadow -Wno-long-long -Wnon-virtual-dtor
mac|darwin: {
    QMAKE_CXXFLAGS += -ansi
} else:false {
    QMAKE_CXXFLAGS += -ansi -Wc++0x-compat
} else {
    QMAKE_CXXFLAGS += -std=c++0x
}


LIBS += -L../../mimetypes -lQtMimeTypes


INCLUDEPATH += ../../../include/QtMimeTypes ../../../src/mimetypes
CONFIG += depend_includepath


SOURCES += mimetypes.cpp

# No headers


SOURCES += qdeclarativemimetype.cpp \
           qdeclarativemimedatabase.cpp

HEADERS += qdeclarativemimetype_p.h \
           qdeclarativemimedatabase_p.h


qmldir.files += $$PWD/qmldir


INSTALLS += qmldir target

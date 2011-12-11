TEMPLATE=app
TARGET=tst_qdeclarativemimetype
#CONFIG += warn_on qmltestcase
QT += qmltest

# runtime environment
LIBS += -L../../../src/mimetypes -lQtMimeTypes


INCLUDEPATH += ../../../includes/QtMimeTypes ../../../src/mimetypes
CONFIG += depend_includepath


SOURCES += tst_qdeclarativemimetype.cpp


# this reads the QML files from the same directory as this pro file
DEFINES += QUICK_TEST_SOURCE_DIR=\"\\\".\\\"\"

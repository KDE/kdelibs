TEMPLATE	= app

include( $(QKW)/kwcommon.pro )

TARGET = makekdewidgets
DESTDIR = .

win32 {
#CONFIG -= console
#CONFIG += windows
}

SOURCES = makekdewidgets.cpp

HEADERS = 




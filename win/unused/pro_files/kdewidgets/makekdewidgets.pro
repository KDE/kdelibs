TEMPLATE	= app

include( $(KDELIBS)/win/common.pro )


TARGET = makekdewidgets
DESTDIR = .

win32 {
#CONFIG -= console
#CONFIG += windows
}

SOURCES = makekdewidgets.cpp

HEADERS = 




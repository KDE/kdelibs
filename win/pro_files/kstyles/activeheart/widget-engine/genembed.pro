TEMPLATE	= app

include( $(QKW)/kwcommon.pro )

win32 {
	CONFIG +=console
	CONIG -=windows
}

DESTDIR=.
TARGET		= genembed

SOURCES = \
genembed.cpp



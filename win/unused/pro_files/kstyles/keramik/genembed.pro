TEMPLATE	= app

include( $(KDELIBS)/win/common.pro )


win32 {
	CONFIG +=console
	CONIG -=windows
}

DESTDIR=.
TARGET		= genembed

SOURCES = \
genembed.cpp



TEMPLATE	= app

include( $(KDELIBS)/win/common.pro )


unix:LIBS += -lkdefx

win32 {
	CONFIG +=console
	CONIG -=windows
}

DESTDIR=.


contains( KW_CONFIG, release ) {
TARGET		= genembed_rel
}
!contains( KW_CONFIG, release ) {
TARGET		= genembed
}

SOURCES = \
genembed.cpp



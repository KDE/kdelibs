TEMPLATE	= lib

include( $(QKW)/kwcommon.pro )

# needed to export library classes:
DEFINES += MAKE_KSSL_LIB
win32:LIBS += $$QKWLIB\kdecore$$KDELIB_SUFFIX $$QKWLIB\kdeui$$KDELIB_SUFFIX

QMAKE_CXXFLAGS += /I$(QKW)/kdelibs/kio/kssl

system( bash kmoc )

TARGET		= kssl$$KDEBUG

SOURCES = \
ksslcertdlg.cc

HEADERS		= \
ksslcertdlg.h

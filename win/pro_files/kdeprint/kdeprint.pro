TEMPLATE	= lib

include( $(QKW)/kwcommon.pro )

# needed to export library classes:
DEFINES += MAKE_KDEPRINT_LIB

TARGET		= kdeprint$$KDEBUG

SOURCES = \
kprinter.cpp \
kpreloadobject.cpp

HEADERS =

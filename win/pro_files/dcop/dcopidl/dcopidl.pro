TEMPLATE	= app

include( $(KDELIBS)/win/common.pro )

# needed to export library classes:

LIBS += $$KDELIBDESTDIR/dcop$$KDELIB_SUFFIX

LIBS -= $$KDELIBDESTDIR/kdecore$$KDELIB_SUFFIX $$KDELIBDESTDIR/kdeui$$KDELIB_SUFFIX \
	$$KDELIBDESTDIR/kdefx$$KDELIB_SUFFIX

TARGET		= dcopidl

DEFINES += YY_ALWAYS_INTERACTIVE

SOURCES = \
main.cpp \
scanner.cc \
yacc.cc

HEADERS		=

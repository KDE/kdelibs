TEMPLATE	= app

include( $(KDELIBS)/win/common.pro )

LIBS -= $$KDELIBDESTDIR/kdecore$$KDELIB_SUFFIX \
	$$KDELIBDESTDIR/kdeui$$KDELIB_SUFFIX $$KDELIBDESTDIR/kdefx$$KDELIB_SUFFIX

TARGET		= dcopidl2cpp

#DEFINES += YY_ALWAYS_INTERACTIVE

SOURCES = \
main.cpp \
skel.cpp \
stubimpl.cpp \
stub.cpp

HEADERS		=

TEMPLATE	= app

include( $(QKW)/kwcommon.pro )

# needed to export library classes:
#win32:LIBS += $$QKWLIB/dcop$$KDELIB_SUFFIX

win32:LIBS -= $$QKWLIB/kdecore$$KDELIB_SUFFIX $$QKWLIB/kdeui$$KDELIB_SUFFIX $$QKWLIB/kdefx$$KDELIB_SUFFIX

TARGET		= dcopidl2cpp

#DEFINES += YY_ALWAYS_INTERACTIVE

SOURCES = \
main.cpp \
skel.cpp \
stubimpl.cpp \
stub.cpp

HEADERS		=

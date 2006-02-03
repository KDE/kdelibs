TEMPLATE	= lib

include( $(KDELIBS)/win/common.pro )

# needed to export library classes:
DEFINES += MAKE_DCOP_LIB

TARGET		= dcop$$KDEBUG

system( bash kmoc )

LIBS += $$KDELIBDESTDIR/kdeice$$KDELIB_SUFFIX

SOURCES = \
dcopref.cpp \
dcopobject.cpp \
dcopclient.cpp \
dcopstub.cpp


TEMPLATE	= lib

include( $(QKW)/kwcommon.pro )

# needed to export library classes:
DEFINES += MAKE_DCOP_LIB

TARGET		= dcop$$KDEBUG

system( bash kmoc )

SOURCES = \
dcopref.cpp \
dcopobject.cpp \
dcopclient.cpp \
dcopstub.cpp \
dcopsignals.cpp

#dcopserver.cpp \

HEADERS		=
#dcopclient.h

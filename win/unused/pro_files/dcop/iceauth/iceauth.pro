TEMPLATE	= app

CONFIG += nokdecore nokdefx nokdeui
include( $(KDELIBS)/win/common.pro )

# needed to export library classes:

LIBS += $$KDELIBDESTDIR/kdeice$$KDELIB_SUFFIX

TARGET		= iceauth

SOURCES = \
iceauth.c \
process.c

HEADERS		=

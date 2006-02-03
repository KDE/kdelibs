TEMPLATE	= app

include( $(KDELIBS)/win/common.pro )

# needed to export library classes:
#DEFINES += MAKE_DCOP_LIB

DEFINES += KDECORE_EXPORT=

TARGET		= dcopref

#system( bash kmoc .. )

#INCLUDEPATH += .. ../moc

LIBS += $$KDELIBDESTDIR/dcop$$KDELIB_SUFFIX $$KDELIBDESTDIR/kdeice$$KDELIB_SUFFIX $$KDELIBDESTDIR/kdecore$$KDELIB_SUFFIX

SOURCES	+= ../dcopref.c

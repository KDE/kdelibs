TEMPLATE	= app

# needed to export library classes:

CONFIG += nokdecore nokdefx nokdeui
include( $(KDELIBS)/win/common.pro )

DEFINES += MAKE_DCOP_LIB 

TARGET		= dcopserver

system( bash kmoc .. )

INCLUDEPATH += .. ../moc

LIBS += $$KDELIBDESTDIR/dcop$$KDELIB_SUFFIX $$KDELIBDESTDIR/kdeice$$KDELIB_SUFFIX

SOURCES	+= ../dcopserver.cpp \
	../dcopsignals.cpp 

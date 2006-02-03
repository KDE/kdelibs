TEMPLATE	= app

CONFIG += nokdecore nokdefx nokdeui
include( $(KDELIBS)/win/common.pro )

# needed to export library classes:
DEFINES += MAKE_DCOP_LIB

TARGET		= dcopserver_shutdown

system( bash kmoc .. )

INCLUDEPATH += .. ../moc

LIBS += $$KDELIBDESTDIR/dcop$$KDELIB_SUFFIX $$KDELIBDESTDIR/kdeice$$KDELIB_SUFFIX

SOURCES = ../dcopserver_shutdown_win.cpp

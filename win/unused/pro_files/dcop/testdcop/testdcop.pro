TEMPLATE = app

include( $(KDELIBS)/win/common.pro )

TARGET = testdcop
DESTDIR = .

system( bash kmoc .. )

INCLUDEPATH += ../moc

LIBS += $$KDELIBDESTDIR/dcop$$KDELIB_SUFFIX $$KDELIBDESTDIR/kdeice$$KDELIB_SUFFIX $$KDELIBDESTDIR/kdecore$$KDELIB_SUFFIX

SOURCES = ../testdcop.cpp

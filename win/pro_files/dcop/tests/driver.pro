TEMPLATE	= app

include( $(KDELIBS)/win/common.pro )

TARGET = driver
DESTDIR = .

system( bash kmoc )
system( bash kdcopidl )

INCLUDEPATH += .. ../moc

LIBS += $$KDELIBDESTDIR/dcop$$KDELIB_SUFFIX $$KDELIBDESTDIR/kdeice$$KDELIB_SUFFIX $$KDELIBDESTDIR/kdecore$$KDELIB_SUFFIX

SOURCES	+= driver.cpp test_stub.cpp

TEMPLATE	= lib

CONFIG += kde3lib #this is a dynamic kde library

include( $(KDELIBS)/win/common.pro )

LIBS += $$KDELIBDESTDIR/kdecore$$KDELIB_SUFFIX $$KDELIBDESTDIR/kio$$KDELIB_SUFFIX


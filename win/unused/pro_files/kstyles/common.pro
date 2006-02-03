TEMPLATE	= lib

CONFIG += kstyle

include( $(KDELIBS)/win/common.pro )


# needed to export library classes:
DEFINES += MAKE_KSTYLE_LIB 

LIBS += $$KDELIBDESTDIR\kdefx$$KDELIB_SUFFIX


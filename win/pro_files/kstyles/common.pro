TEMPLATE	= lib

CONFIG += kstyle

include( $(QKW)/kwcommon.pro )

# needed to export library classes:
DEFINES += MAKE_KSTYLE_LIB 

win32:LIBS += $$QKWLIB\kdefx$$KDELIB_SUFFIX


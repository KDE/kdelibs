TEMPLATE	= lib

include( $(QKW)/kwcommon.pro )

# needed to export library classes:
DEFINES += MAKE_KVCARD_LIB

win32:LIBS += $$QKWLIB\kdecore$$KDELIB_SUFFIX 
#$$QKWLIB\kdeui$$KDELIB_SUFFIX \
#	$$QKWLIB\kresources$$KDELIB_SUFFIX $$QKWLIB\dcop$$KDELIB_SUFFIX $$QKWLIB\kio$$KDELIB_SUFFIX

INCLUDEPATH += $(QKW)/kdelibs/kabc/vcard/include $(QKW)/kdelibs/kabc/vcard/include/generated \
	$(QKW)/kdelibs/kabc/vcardparser

system( bash kmoc )
system( bash kdcopidl )

TARGET		= kvcard$$KDEBUG

SOURCES = \
vCard-all.cpp

HEADERS		=


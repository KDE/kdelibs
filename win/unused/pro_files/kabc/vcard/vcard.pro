TEMPLATE	= lib

include( $(KDELIBS)/win/common.pro )

# needed to export library classes:
DEFINES += MAKE_KVCARD_LIB

LIBS += $$KDELIBDESTDIR\kdecore$$KDELIB_SUFFIX 

INCLUDEPATH += $(KDELIBS)/kabc/vcard/include $(KDELIBS)/kabc/vcard/include/generated \
	$(KDELIBS)/kabc/vcardparser

system( bash kmoc )
system( bash kdcopidl )

TARGET		= kvcard$$KDEBUG

SOURCES = \
vCard-all.cpp

HEADERS		=


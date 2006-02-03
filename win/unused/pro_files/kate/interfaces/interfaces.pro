TEMPLATE	= lib

include( $(KDELIBS)/win/common.pro )

# needed to export library classes:
DEFINES += MAKE_KATEPARTINTERFACES_LIB

TARGET		= katepartinterfaces$$KDEBUG

LIBS += $$KDELIBDESTDIR/kdecore$$KDELIB_SUFFIX $$KDELIBDESTDIR/kdeui$$KDELIB_SUFFIX \
	$$KDELIBDESTDIR/ktexteditor$$KDELIB_SUFFIX $$KDELIBDESTDIR/kparts$$KDELIB_SUFFIX \
	$$KDELIBDESTDIR/ktexteditor$$KDELIB_SUFFIX 

INCLUDEPATH += $(KDELIBS)/interfaces

system( bash kmoc )

SOURCES = \
katecmd.cpp \
interfaces.cpp

HEADERS =

TEMPLATE	= lib

include( $(KDELIBS)/win/common.pro )


# needed to export library classes:
DEFINES += MAKE_KUTILS_LIB

TARGET		= kutils$$KDEBUG

LIBS += $$KDELIBDESTDIR/kdecore$$KDELIB_SUFFIX $$KDELIBDESTDIR/kdeui$$KDELIB_SUFFIX \
	$$KDELIBDESTDIR/kio$$KDELIB_SUFFIX 

INCLUDEPATH += $(KDELIBS)/interfaces/kregexpeditor

system( bash kmoc )

SOURCES = \
kfind.cpp \
kfinddialog.cpp \
kmultitabbar.cpp \
kplugininfo.cpp \
kreplace.cpp \
kreplacedialog.cpp

HEADERS		=

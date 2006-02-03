TEMPLATE	= lib
CONFIG += kde3lib #this is a dynamic kde library

include( $(KDELIBS)/win/common.pro )

system( makekdewidgets -o kdewidgets.cpp kde.widgets )

#no _d because it's a special case

TARGET = kdewidgets

DESTDIR		= $$KDELIBDESTDIR/kde3/plugins/designer

LIBS +=  $$KDELIBDESTDIR/kdecore$$KDELIB_SUFFIX $$KDELIBDESTDIR/kio$$KDELIB_SUFFIX \
	$$KDELIBDESTDIR/kdeui$$KDELIB_SUFFIX \
	$$KDELIBDESTDIR/kabc$$KDELIB_SUFFIX $$KDELIBDESTDIR/kutils$$KDELIB_SUFFIX \
	$$KDELIBDESTDIR/kresources$$KDELIB_SUFFIX 

SOURCES += classpreviews.cpp kdewidgets.cpp


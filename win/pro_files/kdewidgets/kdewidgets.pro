TEMPLATE	= lib
CONFIG += kde3lib #this is a dynamic kde library

include( $(QKW)/kwcommon.pro )

TARGET = kdewidgets

DESTDIR		= $$QKWLIB/kde3/plugins/designer

win32:LIBS +=  $$QKWLIB/kdecore$$KDELIB_SUFFIX $$QKWLIB/kio$$KDELIB_SUFFIX $$QKWLIB/kdeui$$KDELIB_SUFFIX \
$$QKWLIB/kabc$$KDELIB_SUFFIX $$QKWLIB/kutils$$KDELIB_SUFFIX $$QKWLIB/kresources$$KDELIB_SUFFIX 

SOURCES += classpreviews.cpp kdewidgets.cpp


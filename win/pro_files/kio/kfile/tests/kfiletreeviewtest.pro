TEMPLATE	= app


include( $(QKW)/kwcommon.pro )

#allow to select target independently from debug information
kdebase_release:CONFIG -= console
kdebase_release:CONFIG += windows
kdebase_release:QMAKE_MAKEFILE = Makefile.release

win32:LIBS += $$QKWLIB/dcop$$KDELIB_SUFFIX $$QKWLIB/kio$$KDELIB_SUFFIX


TARGET		= kfiletreeviewtest

win32:LIBS +=  $$QKWLIB/kdefx$$KDELIB_SUFFIX \
  $$QKWLIB/kio$$KDELIB_SUFFIX \
  $$QKWLIB/kparts$$KDELIB_SUFFIX

#INCLUDEPATH	+= $(QKW)/kdelibs/interfaces/kregexpeditor

# icon
win32:LIBS+=$(QKW)/files/resources/kfind.res

system( bash kmoc )

SOURCES = \
kfiletreeviewtest.cpp

HEADERS		= 



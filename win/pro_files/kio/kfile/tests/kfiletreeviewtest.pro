TEMPLATE	= app

include( $(KDELIBS)/win/common.pro )


#allow to select target independently from debug information
kdebase_release:CONFIG -= console
kdebase_release:CONFIG += windows
kdebase_release:QMAKE_MAKEFILE = Makefile.release


TARGET		= kfiletreeviewtest

LIBS +=  $$KDELIBDESTDIR/kdefx$$KDELIB_SUFFIX \
  $$KDELIBDESTDIR/kio$$KDELIB_SUFFIX \
  $$KDELIBDESTDIR/kparts$$KDELIB_SUFFIX \
  $$KDELIBDESTDIR/dcop$$KDELIB_SUFFIX $$KDELIBDESTDIR/kio$$KDELIB_SUFFIX

# icon
LIBS+=$(KDELIBS)/win/resources/kfind.res

system( bash kmoc )

SOURCES = \
kfiletreeviewtest.cpp

HEADERS		= 



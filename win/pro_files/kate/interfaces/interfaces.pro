TEMPLATE	= lib

include( $(QKW)/kwcommon.pro )

# needed to export library classes:
DEFINES += MAKE_KATEPARTINTERFACES_LIB

TARGET		= katepartinterfaces$$KDEBUG

win32:LIBS += $$QKWLIB/kdecore$$KDELIB_SUFFIX $$QKWLIB/kdeui$$KDELIB_SUFFIX 
win32:LIBS += $$QKWLIB/ktexteditor$$KDELIB_SUFFIX $$QKWLIB/kparts$$KDELIB_SUFFIX 
win32:LIBS += $$QKWLIB/ktexteditor$$KDELIB_SUFFIX 

INCLUDEPATH += $(QKW)/kdelibs/interfaces

system( bash kmoc )

SOURCES = \
katecmd.cpp \
interfaces.cpp

HEADERS =

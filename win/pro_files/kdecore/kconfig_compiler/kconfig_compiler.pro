TEMPLATE	= app

include( $(QKW)/kwcommon.pro )

TARGET = kconfig_compiler

win32 {
#CONFIG -= console
#CONFIG += windows
}

#win32:LIBS +=  $$QKWLIB/kdefx$$KDELIB_SUFFIX \
#  $$QKWLIB/kio$$KDELIB_SUFFIX \
#  $$QKWLIB/kparts$$KDELIB_SUFFIX

# icon
#win32:LIBS+=$(QKW)/files/resources/kfind.res

#system( bash kmoc )

SOURCES = kconfig_compiler.cpp

HEADERS = 



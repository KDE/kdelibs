TEMPLATE	= lib

DEFINES += MAKE_LTDL_LIB
DEFINES += DLL_EXPORT HAVE_STRING_H=1 HAVE_STDLIB_H=1 HAVE_STDIO_H=1 HAVE_MALLOC_H=1 HAVE_CTYPE_H=1
DEFINES += LTDL_OBJDIR=\".libs/\"

include( $(KDELIBS)/win/common.pro )


TARGET		= ltdl$$KDEBUG


SOURCES = \
ltdl.c \
ltdl_win.c

HEADERS		= \
ltdl.h \
ltdl_win.h

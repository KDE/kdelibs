TEMPLATE	= lib

include( $(KDELIBS)/win/common.pro )


# needed to export library classes:
DEFINES += MAKE_KDEFX_LIB

TARGET		= kdefx$$KDEBUG

system( bash kmoc )

SOURCES = \
kimageeffect.cpp \
kpixmapeffect.cpp \
kpixmapsplitter.cpp \
kpixmap.cpp \
kstyle.cpp \
kdrawutil.cpp \
kcpuinfo.cpp

HEADERS		=

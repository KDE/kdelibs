include( ../../common.pro )

system( bash kmoc )
system( qmake genembed.pro -o Makefile.genembed )
system( nmake /f Makefile.genembed clean )
system( nmake /f Makefile.genembed )
system( bash ./genemb.sh )

TARGET		= activeheart$$KDEBUG

SOURCES = \
colorutil.cpp \
gradients.cpp \
activeheart.cpp \
pixmaploader.cpp



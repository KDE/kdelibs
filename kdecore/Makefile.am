## KDE/lib/core/Makefile.am
##
## (C) 1996 Matthias Kalle Dalheimer
## (C) 1997 Stephan Kulow

MOC = @MOC@ 
INCLUDES=-I@qt_includes@ -I@x_includes@
CLEANFILES = *.moc
VERSION = @VERSION@
EXTRA_DIST = convert_key.c
lib_LTLIBRARIES = libkdecore.la
SUFFIXES = .moc

include_HEADERS = Kconfig.h Kconfigdata.h kcolorgroup.h kapp.h\
	kurl.h ksock.h drag.h kpixmap.h kkeyconf.h kkeydata.h ckey.h kmisc.h

libkdecore_la_SOURCES = Kconfig.cpp kcolorgroup.cpp kapp.cpp kurl.cpp \
	ksock.cpp drag.cpp kpixmap.cpp kkeyconf.cpp

libkdecore_la_LIBADD = @MISCOBJS@
libkdecore_la_DEPENDENCIES = @MISCOBJS@ 
libkdecore_la_LDFLAGS = -version-info 0:7 -rpath $(libdir) 

BUILT_SOURCES = ksock.moc drag.moc kkeydata.moc

%.moc: %.h
	$(MOC) $< -o $@

ckey.h : convert_key.c
	$(CC) $(CFLAGS) convert_key.c -o convert_key
	convert_key $(QTDIR)
	cat ackey ckey >> ckey.h
	rm -f ackey ckey


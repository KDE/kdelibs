## KDE/lib/core/Makefile.am
##
## (C) 1996-1997 Matthias Kalle Dalheimer
## (C) 1997 Stephan Kulow

MOC = @MOC@ 
INCLUDES=@QT_INCLUDES@ @X_INCLUDES@
CLEANFILES = *.moc
VERSION = @VERSION@
EXTRA_DIST = convert_key.c
lib_LTLIBRARIES = libkdecore.la
SUFFIXES = .moc
CXXFLAGS = @CXXFLAGS@ -DKDEDIR=\"@KDEDIR@\"
COMPILE = $(CC) $(DEFS) $(INCLUDES) $(CPPFLAGS) $(CFLAGS)

include_HEADERS = Kconfig.h Kconfigdata.h kcolorgroup.h kapp.h\
	kurl.h ksock.h drag.h kkeyconf.h kkeydata.h ckey.h kmisc.h

libkdecore_la_SOURCES = Kconfig.cpp kcolorgroup.cpp kapp.cpp kurl.cpp \
	ksock.cpp drag.cpp kkeyconf.cpp

libkdecore_la_LIBADD = @MISCOBJS@
libkdecore_la_DEPENDENCIES = @MISCOBJS@ 
libkdecore_la_LDFLAGS = -version-info 0:9 -rpath $(libdir) 

BUILT_SOURCES = ksock.moc drag.moc kkeydata.moc kapp.moc

%.moc: %.h
	$(MOC) $< -o $@

ckey.h : convert_key.c
	$(CC) $(CFLAGS) convert_key.c -o convert_key
	convert_key $(QTDIR)
	cat ackey ckey >> ckey.h
	rm -f ackey ckey


## KDE/lib/core/Makefile.am
##
## (C) 1996-1997 Matthias Kalle Dalheimer
## (C) 1997 Stephan Kulow

MOC = @MOC@ 
INCLUDES=@QT_INCLUDES@ @X_INCLUDES@
CLEANFILES = $(BUILT_SOURCES)
VERSION = @VERSION@
EXTRA_DIST = convert_key.c
lib_LTLIBRARIES = libkdecore.la
SUFFIXES = .moc
CXXFLAGS = @CXXFLAGS@ -DKDEDIR=\"@KDEDIR@\"

include_HEADERS = Kconfig.h Kconfigdata.h kcolorgroup.h kapp.h\
	kurl.h ksock.h drag.h kkeyconf.h kkeydata.h ckey.h kmisc.h	\
	kprocess.h kprocctrl.h klocale.h kiconloader.h kdebug.h	\
	kstdaccel.h kwm.h

noinst_HEADERS = kdebugdialog.h

libkdecore_la_SOURCES = Kconfig.cpp kcolorgroup.cpp kapp.cpp kurl.cpp \
	ksock.cpp drag.cpp kkeyconf.cpp	kprocess.cpp kprocctrl.cpp \
	klocale.cpp libintl.cpp setenv.cpp kiconloader.cpp kdebug.cpp	\
	kstdaccel.cpp kwm.cpp

libkdecore_la_LDFLAGS = -version-info 0:11 @QT_LDFLAGS@ @X_LDFLAGS@ 
libkdecore_la_LIBADD = -lqt -lX11 -lXext

BUILT_SOURCES = ksock.moc drag.moc kkeydata.moc kapp.moc kprocess.moc	\
	kprocctrl.moc kiconloader.moc kdebugdialog.moc

ksock.cpp: ksock.moc
drag.cpp: drag.moc
kkeyconf.cpp: kkeydata.moc
kapp.cpp: kapp.moc
kprocess.cpp: kprocess.moc
kprocctrl.cpp: kprocctrl.moc
kiconloader.cpp: kiconloader.moc
kdebug.cpp: kdebugdialog.moc
kstdaccel.cpp: kstdaccel.h

ckey.h : convert_key.c
	$(CC) $(CFLAGS) convert_key.c -o convert_key
	./convert_key $(QTDIR)
	cat ackey ckey > ckey.h
	rm -f ackey ckey

ksock.moc: ksock.h
	$(MOC) ksock.h -o ksock.moc

drag.moc: drag.h
	$(MOC) drag.h -o drag.moc

kkeydata.moc: kkeydata.h
	$(MOC) kkeydata.h -o kkeydata.moc

kapp.moc: kapp.h
	$(MOC) kapp.h -o kapp.moc

kprocess.moc: kprocess.h
	$(MOC) kprocess.h -o kprocess.moc

kprocctrl.moc: kprocctrl.h
	$(MOC) kprocctrl.h -o kprocctrl.moc

kiconloader.moc: kiconloader.h
	$(MOC) kiconloader.h -o kiconloader.moc

kdebugdialog.moc: kdebugdialog.h
	$(MOC) kdebugdialog.h -o kdebugdialog.moc

kstdaccel.moc: kstdaccel.h
	$(MOC) kstdaccel.h -o kstdaccel.moc

## KDE/lib/core/Makefile.am
##
## (C) 1996-1997 Matthias Kalle Dalheimer
## (C) 1997 Stephan Kulow

MOC = @MOC@ 
INCLUDES=@QT_INCLUDES@ @X_INCLUDES@
DISTCLEANFILES = $(BUILT_SOURCES)
VERSION = @VERSION@
EXTRA_DIST = convert_key.c
lib_LTLIBRARIES = libkdecore.la
SUFFIXES = .moc
CXXFLAGS = @CXXFLAGS@ -DKDEDIR=\"@KDEDIR@\"

include_HEADERS = kconfig.h kconfigdata.h ksimpleconfig.h kconfigbase.h \
	kcolorgroup.h kapp.h\
	kurl.h ksock.h drag.h kkeyconf.h kkeydata.h ckey.h kmisc.h	\
	kprocess.h kprocctrl.h klocale.h kiconloader.h kdebug.h	\
	kstdaccel.h kwm.h krootprop.h

noinst_HEADERS = kdebugdialog.h

libkdecore_la_SOURCES = kconfig.cpp kconfigbase.cpp ksimpleconfig.cpp \
	kcolorgroup.cpp kapp.cpp kurl.cpp \
	ksock.cpp drag.cpp kkeyconf.cpp	kprocess.cpp kprocctrl.cpp \
	klocale.cpp libintl.cpp fakes.cpp kiconloader.cpp kdebug.cpp	\
	kstdaccel.cpp kwm.cpp krootprop.cpp

libkdecore_la_LDFLAGS = -version-info 0:11 @QT_LDFLAGS@ @X_LDFLAGS@ 
libkdecore_la_LIBADD = -lqt -lX11 -lXext

BUILT_SOURCES = ksock.moc drag.moc kkeydata.moc kapp.moc kprocess.moc	\
	kprocctrl.moc kiconloader.moc kdebugdialog.moc kconfig.moc \
	kconfigbase.moc ksimpleconfig.moc

ksock.cpp: ksock.moc
drag.cpp: drag.moc
kkeyconf.cpp: kkeydata.moc
kapp.cpp: kapp.moc
kprocess.cpp: kprocess.moc
kprocctrl.cpp: kprocctrl.moc
kiconloader.cpp: kiconloader.moc
kdebug.cpp: kdebugdialog.moc
kconfigbase.cpp: kconfigbase.moc
kconfig.cpp: kconfig.moc
ksimpleconfig.cpp: ksimpleconfig.moc

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

kconfig.moc: kconfig.h
	$(MOC) kconfig.h -o kconfig.moc

kconfigbase.moc: kconfigbase.h
	$(MOC) kconfigbase.h -o kconfigbase.moc

ksimpleconfig.moc: ksimpleconfig.h
	$(MOC) ksimpleconfig.h -o ksimpleconfig.moc

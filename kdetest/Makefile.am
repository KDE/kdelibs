## KDE/src/test/Makefile.am
##
## (C) 1996 Matthias Kalle Dalheimer
## (C) 1997 Stephan Kulow

MOC = @MOC@
INCLUDES = -I$(topdir)/kdeui -I$(topdir)/kdecore -I@qt_includes@ -I@x_includes@

CLEANFILES = *.moc

noinst_PROGRAMS = kconfigtest kledlamptest kprogresstest kurltest kpannertest \
					kcolordlgtest kdatepicktest kfontdialogtest	\
					kmsgboxtest kpopuptest ktabctltest ktreelisttest	\
					kkeyconfigtest kseparatortest \
					kiconloadertest

noinst_HEADERS = kdatepicktestdlg.h kmsgboxtest.h ktreelisttest.h \
					kkeyconfigtest.h ktabctltest.h \
					kiconloadertestmain.h
BUILT_SOURCES = kconfigtest.moc kpannertest.moc ktabctltest.moc \
				kmsgboxtest.moc kdatepicktestdlg.moc ktreelisttest.moc	\
				kkeyconfigtest.moc kseparatortest.moc 	\
				kiconloadertestmain.moc

kconfigtest_SOURCES = kconfigtest.cpp
kledlamptest_SOURCES = kledlamptest.cpp
kprogresstest_SOURCES = kprogresstest.cpp
kurltest_SOURCES = kurltest.cpp
kpannertest_SOURCES = kpannertest.cpp
kcolordlgtest_SOURCES = kcolordlgtest.cpp
kdatepicktest_SOURCES = kdatepicktest.cpp kdatepicktestdlg.cpp
kfontdialogtest_SOURCES = kfontdialogtest.cpp
kmsgboxtest_SOURCES = kmsgboxtest.cpp
kpopuptest_SOURCES = kpopuptest.cpp
ktabctltest_SOURCES = ktabctltest.cpp
ktreelisttest_SOURCES = ktreelisttest.cpp
kkeyconfigtest_SOURCES = kkeyconfigtest.cpp
kseparatortest_SOURCES = kseparatortest.cpp
kiconloadertest_SOURCES = kiconloadertest.cpp kiconloadertestmain.cpp

SUFFIXES = .moc
LDADD =  -L$(topdir)/kdecore/.libs -L$(topdir)/kdeui/.libs -lkdeui -lkdecore @X_LDFLAGS@ -lX11 -lXext @QT_LDFLAGS@ -lqt

%.moc: %.h
	$(MOC) $< -o $@

.cpp.moc:
	$(MOC) $< -o $@	


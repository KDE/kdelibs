## KDE/kdelibs/kdetest/Makefile.am
##
## (C) 1996 Matthias Kalle Dalheimer
## (C) 1997 Stephan Kulow

MOC = @MOC@
INCLUDES = -I$(topdir)/kdeui -I$(topdir)/kdecore -I@qt_includes@ -I@x_includes@

LDADD = ../kdecore/libkdecore.la ../kdeui/libkdeui.la @QT_LDFLAGS@ -lqt @X_LDFLAGS@ -lX11 -lXext

DISTCLEANFILES = *.moc

noinst_PROGRAMS = kconfigtest kledlamptest kprogresstest kurltest \
		kcolordlgtest kdatepicktest kfontdialogtest	\
		kmsgboxtest kpopuptest ktabctltest ktreelisttest	\
		kkeyconfigtest kseparatortest kiconloadertest		\
		kwindowtest klinetest kstatusbartest kspinboxtest	\
		kprocesstest kpannertest kbuttonboxtest ksimpleconfigtest

noinst_HEADERS = kdatepicktestdlg.h kmsgboxtest.h ktreelisttest.h \
		kkeyconfigtest.h ktabctltest.h kiconloadertestmain.h	\
		kwindowtest.h klinetest.h kstatusbartest.h kspinboxtest.h	\
		kprocesstest.h

BUILT_SOURCES = ktabctltest.moc kmsgboxtest.moc kdatepicktestdlg.moc \
		ktreelisttest.moc kkeyconfigtest.moc kiconloadertestmain.moc \
		kconfigtest.moc kpannertest.moc kwindowtest.moc klinetest.moc \
		kstatusbartest.moc kspinboxtest.moc kprocesstest.moc 

ktabctltest.cpp: ktabctltest.moc
kiconloadertestmain.cpp: kiconloadertestmain.moc
kmsgboxtest.cpp: kmsgboxtest.moc
kdatepicktestdlg.cpp: kdatepicktestdlg.moc
ktreelisttest.cpp: ktreelisttest.moc
kkeyconfigtest.cpp: kkeyconfigtest.moc
kconfigtest.cpp: kconfigtest.moc
kpannertest.cpp: kpannertest.moc
kwindowtest.cpp: kwindowtest.moc
klinetest.cpp: klinetest.moc
kstatusbartest.cpp: kstatusbartest.moc
kspinboxtest.cpp: kspinboxtest.moc
kprocesstest.cpp: kprocesstest.moc

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
kwindowtest_SOURCES = kwindowtest.cpp
klinetest_SOURCES = klinetest.cpp
kstatusbartest_SOURCES = kstatusbartest.cpp
kspinboxtest_SOURCES = kspinboxtest.cpp
kprocesstest_SOURCES = kprocesstest.cpp
kbuttonboxtest_SOURCES = kbuttonboxtest.cpp
ksimpleconfigtest_SOURCES = ksimpleconfigtest.cpp

klinetest.moc: klinetest.h
	$(MOC) klinetest.h -o klinetest.moc

kspinboxtest.moc: kspinboxtest.h
	$(MOC) kspinboxtest.h -o kspinboxtest.moc

ktabctltest.moc: ktabctltest.h
	$(MOC) ktabctltest.h -o ktabctltest.moc

kmsgboxtest.moc: kmsgboxtest.h
	$(MOC) kmsgboxtest.h -o kmsgboxtest.moc

kdatepicktestdlg.moc: kdatepicktestdlg.h
	$(MOC) kdatepicktestdlg.h -o kdatepicktestdlg.moc

ktreelisttest.moc: ktreelisttest.h
	$(MOC) ktreelisttest.h -o ktreelisttest.moc

kwindowtest.moc: kwindowtest.h
	$(MOC) kwindowtest.h -o kwindowtest.moc

kpannertest.moc: kpannertest.h
	$(MOC) kpannertest.h -o kpannertest.moc

kkeyconfigtest.moc: kkeyconfigtest.h
	$(MOC) kkeyconfigtest.h -o kkeyconfigtest.moc

kiconloadertestmain.moc: kiconloadertestmain.h
	$(MOC) kiconloadertestmain.h -o kiconloadertestmain.moc

kstatusbartest.moc: kstatusbartest.h
	$(MOC) kstatusbartest.h -o kstatusbartest.moc

kconfigtest.moc: kconfigtest.h
	$(MOC) kconfigtest.h -o kconfigtest.moc

kprocesstest.moc: kprocesstest.h
	$(MOC) kprocesstest.h -o kprocesstest.moc


INCLUDES = -I../client -I../config $(all_includes) -I../client

bin_PROGRAMS = knotifytest

knotifytest_SOURCES = main.cpp knotifytestwindow.cpp knotifytestview.ui 
knotifytest_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_KDEUI) $(LIB_KDECORE) $(LIB_KIO)  $(LIB_QT)
knotifytest_LDADD = ../config/libknotifyconfig.la

noinst_HEADERS = knotifytestwindow.h
METASOURCES =	AUTO

rcdir = $(kde_datadir)/knotifytest
rc_DATA = knotifytestui.rc knotifytest.notifyrc

messages: rc.cpp
	$(XGETTEXT) rc.cpp *.cpp -o $(podir)/knotifytest.pot



INCLUDES= $(all_includes)

CXXFLAGS := $(CXXFLAGS) -Wall @NOREPO@

lib_LTLIBRARIES = libksycoca.la
libksycoca_la_LDFLAGS = -version-info 1:0
libksycoca_la_LIBADD = ../kdeui/libkdeui.la

bin_PROGRAMS = ksycocatest
ksycocatest_LDADD = libksycoca.la
ksycocatest_LDFLAGS = $(all_libraries) $(KDE_RPATH)

libksycoca_la_SOURCES = \
	ksycoca.cpp ksycocadict.cpp ksycocaentry.cpp \
        kservicetypefactory.cpp kservicetype.cpp \
	kmimetype.cpp kmimemagic.cpp \
	kautomount.cc kdirwatch.cc kio_base.cc kio_cache.cc \
	kio_connection.cc kio_dlg.cc kio_error.cc kio_filter.cc \
	kio_interface.cc kio_job.cc kio_listprogress_dlg.cc \
	kio_littleprogress_dlg.cc kio_netaccess.cc kio_openwith.cc \
	kio_pass_dlg.cc kio_paste.cc kio_rename_dlg.cc kio_simpleprogress_dlg.cc \
	kio_skip_dlg.cc kpixmapcache.cc krun.cc kservice.cc ktar.cc \
	kuserpaths.cc kuserprofile.cc 

ksycocatest_SOURCES = main.cpp

ksycoca_METASOURCES = AUTO
libksycoca_la_METASOURCES = AUTO

include_HEADERS = ksycoca.h ksycocafactory.h ksycocaentry.h ksycocadict.h \
	kservicetype.h kservicetypefactory.h \
	kmimetype.h kmimemagic.h kservice.h \
        krun.h kdirwatch.h \
	kio_connection.h kio_base.h kio_rename_dlg.h \
        kio_filter.h kio_dlg.h kio_skip_dlg.h  kio_interface.h \
        kio_pass_dlg.h kautomount.h \
        kio_simpleprogress_dlg.h kio_littleprogress_dlg.h kio_listprogress_dlg.h \
        kio_cache.h kio_netaccess.h kpixmapcache.h kuserprofile.h \
        kio_error.h kio_job.h \
        kio_paste.h kio_openwith.h kuserpaths.h \
        ktar.h
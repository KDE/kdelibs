
INCLUDES= $(all_includes)

CXXFLAGS := $(CXXFLAGS) -Wall @NOREPO@

lib_LTLIBRARIES = libksycoca.la
libksycoca_la_LDFLAGS = -version-info 1:0
libksycoca_la_LIBADD = ../kdecore/libkdecore.la

bin_PROGRAMS = ksycocatest
ksycocatest_LDADD = libksycoca.la
ksycocatest_LDFLAGS = $(all_libraries) $(KDE_RPATH)

libksycoca_la_SOURCES = \
	ksycoca.cpp ksycocadict.cpp ksycocaentry.cpp \
        kservicetypefactory.cpp kservicetype.cpp \
	kmimetype.cpp kmimemagic.cpp

ksycocatest_SOURCES = main.cpp

ksycoca_METASOURCES = AUTO
libksycoca_la_METASOURCES = AUTO

# should be include_HEADERS when ready
noinst_HEADERS = ksycoca.h ksycocafactory.h ksycocaentry.h ksycocadict.h \
	kservicetype.h kservicetypefactory.h \
	kmimetype.h kmimemagic.h


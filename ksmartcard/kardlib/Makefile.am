INCLUDES=$(all_includes) $(PCSC_INCLUDES) -I$(top_srcdir)/kio -I../kardimpl


lib_LTLIBRARIES = libksmartcard.la
METASOURCES= AUTO

include_HEADERS = kcardreader.h			\
		  kcardstatus.h			\
		  kpcsc.h			\
		  kcarddb.h			\
		  kcardfactory.h		\
	          kcardimplementation.h

noinst_HEADERS = 

libksmartcard_la_SOURCES = \
				kpcsc.cc		\
				kcarddb.cc		\
				kcardstatus.cc		\
				kcardreader.cc		\
				kcardfactory.cc		\
				kcardimplementation.cc

libksmartcard_la_LDFLAGS = $(all_libraries) $(PCSC_LDFLAGS) -version-info 1:0 -no-undefined
libksmartcard_la_LIBADD = $(LIB_KDEUI) $(LIB_KIO)
libksmartcard_la_DEPENDENCIES = $(LIB_KDEUI) $(LIB_KIO)

libksmartcard_la_METASOURCES = AUTO

SUBDIRS = 

bin_PROGRAMS = testkdepcsc
testkdepcsc_SOURCES = teste.cc
testkdepcsc_LDADD = $(LIB_KDEUI) -lksmartcard


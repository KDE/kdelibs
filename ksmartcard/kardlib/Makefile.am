INCLUDES= $(PCSC_INCLUDES) -I$(top_srcdir)/kio -I../kardimpl $(all_includes)


lib_LTLIBRARIES = libksmartcard.la
METASOURCES= AUTO

include_HEADERS = kcardreader.h			\
		  kcardstatus.h			\
		  kpcsc.h			\
		  kcarddb.h			\
		  kcardfactory.h		\
	          kcardimplementation.h

libksmartcard_la_SOURCES = \
				kpcsc.cc		\
				kcarddb.cc		\
				kcardstatus.cc		\
				kcardreader.cc		\
				kcardfactory.cc		\
				kcardimplementation.cc

libksmartcard_la_LDFLAGS = $(all_libraries) $(PCSC_LDFLAGS) -version-info 1:0 -no-undefined
libksmartcard_la_LIBADD = $(top_builddir)/kio/libkio.la

bin_PROGRAMS = testkdepcsc
testkdepcsc_SOURCES = teste.cc
testkdepcsc_LDADD = libksmartcard.la


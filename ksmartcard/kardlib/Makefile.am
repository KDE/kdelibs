INCLUDES=$(all_includes) $(PCSC_INCLUDES)

lib_LTLIBRARIES = libksmartcard.la
METASOURCES= AUTO

include_HEADERS = kcardreader.h \
		  kcardstatus.h \
		  kpcsc.h \
		  kcardfactory.h \
	          kcardimplementation.h \
		  kcardgemsafe_impl.cc \
	          kcardgsmimplementation.h

noinst_HEADERS = 

libksmartcard_la_SOURCES = \
				kpcsc.cc  \
				kcardstatus.cc \
				kcardreader.cc \
				kcardfactory.cc \
				kcardimplementation.cc \
				kcardgemsafe_impl.cc \
				kcardgsmimplementation.cc


libksmartcard_la_LDFLAGS = $(all_libraries) $(PCSC_LDFLAGS) -version-info 1:0 -no-undefined
libksmartcard_la_LIBADD = $(LIB_KDEUI) 
libksmartcard_la_DEPENDENCIES = $(LIB_KDEUI)

libksmartcard_la_METASOURCES = AUTO

SUBDIRS = 


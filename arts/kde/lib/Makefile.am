INCLUDES                    =   $(all_includes) -I$(top_srcdir)/arts \
			        -I$(top_srcdir)/arts/mcop -I$(top_srcdir)/arts/flow \
				-I$(top_builddir)/arts/flow -I$(top_srcdir)/arts/soundserver \
				-I$(top_builddir)/arts/soundserver
lib_LTLIBRARIES  	    =	libartskwrapper.la
libartskwrapper_la_SOURCES  =	kwrapper.cc kwrapper_impl.cc
libartskwrapper_la_LDFLAGS  =   -avoid-version -module $(all_libraries)

kwrapper.cc: kwrapper.h
kwrapper.mcoptype: kwrapper.h
kwrapper.mcopclass: kwrapper.h
kwrapper.h: $(top_srcdir)/arts/kde/lib/kwrapper.idl $(MCOPIDL)
	    $(MCOPIDL) -I$(top_srcdir)/arts/soundserver -I$(top_srcdir)/arts/flow -t $(MCOPINC) $(top_srcdir)/arts/kde/lib/kwrapper.idl

DISTCLEANFILES = kwrapper.cc kwrapper.h kwrapper.mcoptype kwrapper.mcopclass

mcoptypedir = $(libdir)/mcop/Arts
mcoptype_DATA = KWrapperFactory.mcopclass

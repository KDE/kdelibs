INCLUDES                    =   $(all_includes) -I$(top_srcdir)/arts \
			        -I$(top_srcdir)/arts/mcop -I$(top_srcdir)/arts/flow \
				-I$(top_builddir)/arts/flow -I$(top_srcdir)/arts/soundserver \
				-I$(top_builddir)/arts/soundserver
lib_LTLIBRARIES  	    =	libartskde.la
libartskde_la_SOURCES	    =	kwrapper.cc kwrapper_impl.cc kplayobject.cc kplayobjectfactory.cpp
libartskde_la_LIBADD        =   $(LIB_KDECORE) $(LIB_KIO) $(LIB_KSYCOCA)
libartskde_la_LDFLAGS       =   -avoid-version -module $(all_libraries)
libartskde_la_METASOURCES   =	AUTO

artskdeincludedir	    =	$(includedir)/arts
artskdeinclude_HEADERS      = 	kplayobject.h kplayobjectfactory.h

kwrapper.cc: kwrapper.h
kwrapper.mcoptype: kwrapper.h
kwrapper.mcopclass: kwrapper.h
kwrapper.h: $(top_srcdir)/arts/kde/kwrapper.idl $(MCOPIDL)
	    $(MCOPIDL) -I$(top_srcdir)/arts/soundserver -t $(MCOPINC) $(top_srcdir)/arts/kde/kwrapper.idl

DISTCLEANFILES = kwrapper.cc kwrapper.h kwrapper.mcoptype kwrapper.mcopclass

mcoptypedir = $(libdir)/mcop/Arts
mcoptype_DATA = KWrapperFactory.mcopclass

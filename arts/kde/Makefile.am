SUBDIRS			    =	lib .
INCLUDES                    =   $(all_includes) -I$(top_srcdir)/arts \
			        -I$(top_srcdir)/arts/mcop -I$(top_srcdir)/arts/flow \
				-I$(top_builddir)/arts/flow -I$(top_srcdir)/arts/soundserver \
				-I$(top_builddir)/arts/soundserver -I$(top_srcdir)/arts/kde/lib
lib_LTLIBRARIES  	    =	libartskde.la
libartskde_la_SOURCES	    =	kplayobject.cc kplayobjectfactory.cpp kartsfloatwatch.cpp
libartskde_la_LIBADD        =   $(LIB_KDECORE) $(LIB_KIO) $(LIB_KSYCOCA)
libartskde_la_LDFLAGS       =   -avoid-version -module $(all_libraries)
libartskde_la_METASOURCES   =	AUTO

artskdeincludedir	    =	$(includedir)/arts
artskdeinclude_HEADERS      = 	kplayobject.h kplayobjectfactory.h kartsfloatwatch.h

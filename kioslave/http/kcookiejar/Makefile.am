# $Id$
# Makefile.am of kdebase/kioslave/http

INCLUDES=	$(all_includes) 
LDFLAGS =       $(all_libraries) $(KDE_RPATH)
LDADD	=	$(LIB_KIO) 

####### Files

bin_PROGRAMS = kcookiejar

kcookiejar_SOURCES = kcookiejar.cpp kcookiejar.h 	\
		     kcookieserver.cpp kcookieserver.h	\
		     kcookiewin.cpp kcookiewin.h main.cpp

kcookiejar_METASOURCES = AUTO


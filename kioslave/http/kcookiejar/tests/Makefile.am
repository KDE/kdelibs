# $Id: Makefile.am 362284 2004-11-11 18:14:09Z waba $
# Makefile.am of kdebase/kioslave/http

INCLUDES=	$(all_includes) 

####### Files

check_PROGRAMS = kcookiejartest

kcookiejartest_SOURCES = kcookiejartest.cpp 
kcookiejartest_LDADD   = $(LIB_KIO)
kcookiejartest_LDFLAGS = $(all_libraries) $(KDE_RPATH)

check-local: kcookiejartest
	./kcookiejartest $(srcdir)/cookie.test
	./kcookiejartest $(srcdir)/cookie_rfc.test
	./kcookiejartest $(srcdir)/cookie_saving.test
	./kcookiejartest $(srcdir)/cookie_settings.test

## Makefile.am of kdebase/kioslave/ftp

INCLUDES=	$(all_includes)
LDFLAGS =       $(all_libraries) $(KDE_RPATH)
LDADD	=	$(LIB_KIO)

####### Files

bin_PROGRAMS = kio_ftp

kio_ftp_SOURCES = main.cc ftp.cc
noinst_HEADERS = main.h ftp.h

kioconfig_DATA = ftp.kdelnk
kioconfigdir = $(kde_confdir)/protocols

messages:


## Makefile.am of kdebase/kioslave/file

INCLUDES=	$(all_includes)
LDFLAGS =       $(all_libraries) $(KDE_RPATH)
LDADD	=	$(LIB_KIO)

####### Files

bin_PROGRAMS = kio_file

kio_file_SOURCES = file.cc
noinst_HEADERS = file.h

kioconfig_DATA = file.desktop
kioconfigdir = $(kde_confdir)/protocols

messages:


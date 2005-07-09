lib_LTLIBRARIES = libkscreensaver.la
libkscreensaver_la_SOURCES = main.cpp kscreensaver.cpp
libkscreensaver_la_LIBADD  = $(LIB_KIO)
if undefined_symbols_allowed
libkscreensaver_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -version-info 6:0:2
else
libkscreensaver_la_LDFLAGS = $(all_libraries) -static
endif 

include_HEADERS = kscreensaver.h kscreensaver_vroot.h

INCLUDES = $(all_includes)
METASOURCES = AUTO

messages:
	$(XGETTEXT) *.cpp *.h -o $(podir)/libkscreensaver.pot 

include $(top_srcdir)/admin/Doxyfile.am

lib_LTLIBRARIES = libkscreensaver.la
libkscreensaver_la_SOURCES = main.cpp kscreensaver.cpp
libkscreensaver_la_LIBADD  = $(LIB_KIO)
libkscreensaver_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -version-info 4:0

include_HEADERS = kscreensaver.h kscreensaver_vroot.h

INCLUDES = $(all_includes)
METASOURCES = AUTO

messages:
	$(XGETTEXT) *.cpp *.h -o $(podir)/libkscreensaver.pot 



AM_CPPFLAGS = -DQT_PLUGIN

INCLUDES = -I$(top_srcdir)/kdefx -I$(top_builddir)/kstyles/keramik/ $(all_includes)
# qembed's output needs that...
KDE_CXXFLAGS = -UQT_NO_ASCII_CAST
noinst_HEADERS = keramik.h pixmaploader.h keramikimage.h bitmaps.h
kde_style_LTLIBRARIES = keramik.la
keramik_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
keramik_la_LIBADD = ../../kdefx/libkdefx.la
keramik_la_SOURCES = keramik.cpp pixmaploader.cpp
keramik_la_METASOURCES = AUTO

noinst_PROGRAMS = genembed

genembed_SOURCES = genembed.cpp
genembed_LDADD = ../../kdefx/libkdefx.la
genembed_LDFLAGS = $(all_libraries) $(KDE_RPATH)

$(srcdir)/pixmaploader.cpp $(srcdir)/keramik.cpp: $(top_builddir)/kstyles/keramik/pixmaps.keramik

$(top_builddir)/kstyles/keramik/pixmaps.keramik: genembed
	pics=`ls $(srcdir)/pics/*.png 2>/dev/null` ;\ 
	$(top_builddir)/kstyles/keramik/genembed $$pics > pixmaps.keramik

.PHONY: make-embed


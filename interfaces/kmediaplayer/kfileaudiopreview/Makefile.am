INCLUDES = -I$(top_srcdir) $(all_includes)
METASOURCES = AUTO

kde_module_LTLIBRARIES = kfileaudiopreview.la

kfileaudiopreview_la_SOURCES = kfileaudiopreview.cpp
kfileaudiopreview_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -module -avoid-version -no-undefined
kfileaudiopreview_la_LIBADD = $(top_builddir)/phonon/ui/libphononui.la $(LIB_KIO)

noinst_HEADERS = kfileaudiopreview.h

messages:
	$(XGETTEXT) $(kfileaudiopreview_la_SOURCES) -o $(podir)/kfileaudiopreview.pot

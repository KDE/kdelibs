
AM_CPPFLAGS = -DQT_PLUGIN

INCLUDES = -I$(top_srcdir)/kdefx $(all_includes)
# qembed's output needs that...
KDE_CXXFLAGS = -UQT_NO_ASCII_CAST
noinst_HEADERS = keramik.h pixmaploader.h
kde_style_LTLIBRARIES = keramik.la
keramik_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
keramik_la_LIBADD = ../../kdefx/libkdefx.la
keramik_la_SOURCES = keramik.cpp pixmaploader.cpp
keramik_la_METASOURCES = AUTO

$(srcdir)/pixmaploader.cpp: pixmaps.embed

QEMBED = $(QTDIR)/tools/qembed/qembed

make-embed:
	pics=`ls $(srcdir)/pics/*.png 2>/dev/null` ;\ 
	$(QEMBED) --images $$pics | \
	sed -e 's,static QDict<QImage> dict;,,' | \
	sed -e 's,dict\.find,imageDict.find,' > $(srcdir)/pixmaps.embed

.PHONY: make-embed

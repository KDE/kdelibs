
INCLUDES= -I$(top_srcdir)/kio -I$(top_srcdir) -I$(top_srcdir)/kio/gzip -I$(top_srcdir)/kio/bzip2 $(LIBXML_CFLAGS) $(LIBXSLT_CFLAGS) $(all_includes)
LDFLAGS = $(KDE_RPATH) $(LIBXML_RPATH) $(LIBXSLT_RPATH) $(all_libraries)
AM_CPPFLAGS = -DSRCDIR="\"$(shell (cd $(srcdir) && pwd))\"" 

FILTER_LIBS = $(LIB_KSYCOCA) $(LIBBZ2)
FILTER_sources = kgzipfilter.cpp kbzip2filter.cpp

####### Files

kde_module_LTLIBRARIES = kio_help.la

kio_help_la_SOURCES = $(FILTER_sources) kio_help.cpp main.cpp xslt.cpp
kio_help_la_LIBADD = $(FILTER_LIBS) $(LIBXSLT_LIBS)
kio_help_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
noinst_HEADERS = kio_help.h

kdelnk_DATA = help.protocol
kdelnkdir = $(kde_servicesdir)

bin_PROGRAMS = meinproc xml2man
meinproc_LDADD = xslt_pure.o $(LIBXSLT_LIBS) $(LIBXML_LIBS) $(FILTER_LIBS)
meinproc_SOURCES = meinproc.cpp $(FILTER_sources)

xml2man_LDADD = xslt_pure.o $(LIBXSLT_LIBS) $(LIBXML_LIBS) $(FILTER_LIBS)
xml2man_SOURCES = xml2man.cpp $(FILTER_sources)

xslt_pure.o: xslt.cpp
	-rm -f xslt_pure.cpp
	$(LN_S) $(srcdir)/xslt.cpp xslt_pure.cpp
	$(CXXCOMPILE) -DSIMPLE_XSLT -c xslt_pure.cpp
	-rm -f xslt_pure.cpp

bin_SCRIPTS = xmlizer kdb2html checkXML

METASOURCES = AUTO

messages:
	$(XGETTEXT) *.cpp -o $(podir)/kdebase/kio_help.pot

install-data-local:
	@(cd $(srcdir) && \
	for subdir in customization docbook; do \
	   for dir in `find $$subdir -type d ! -name CVS `; do \
		$(mkinstalldirs) $(DESTDIR)$(kde_datadir)/ksgmltools2/$$dir ;\
		for file in `ls -1 $$dir/*`; do \
		    if test -f $$file; then \
			$(INSTALL_DATA) $$file  $(DESTDIR)$(kde_datadir)/ksgmltools2/$$file; fi ;\
	   	done ;\
	   done ;\
	done )

uninstall-local:
	-rm -rf $(DESTDIR)$(kde_datadir)/ksgmltools2/customization	
	-rm -rf $(DESTDIR)$(kde_datadir)/ksgmltools2/docbook

kgzipfilter.cpp: 
	$(LN_S) $(srcdir)/../kio/gzip/kgzipfilter.cpp .

kbzip2filter.cpp:
	$(LN_S) $(srcdir)/../kio/bzip2/kbzip2filter.cpp .

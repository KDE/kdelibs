SUBDIRS = libxslt

INCLUDES= $(all_includes) -I$(top_srcdir)/kio -I$(top_srcdir) $(LIBXML_CFLAGS)
LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIBXML_RPATH)
AM_CPPFLAGS = -DSRCDIR="\"$(shell (cd $(srcdir) && pwd))\"" 

####### Files

kde_module_LTLIBRARIES = kio_help.la

kio_help_la_SOURCES = kio_help.cpp main.cpp xslt.cpp
kio_help_la_LIBADD = $(LIB_KSYCOCA) libxslt/libxslt.la
kio_help_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
noinst_HEADERS = kio_help.h

kdelnk_DATA = help.protocol
kdelnkdir = $(kde_servicesdir)

bin_PROGRAMS = meinproc saxon checkXML xml2man
meinproc_LDADD = xslt_pure.o libxslt/libxslt.la $(LIB_KSYCOCA)
meinproc_SOURCES = meinproc.cpp

xml2man_LDADD = xslt_pure.o libxslt/libxslt.la $(LIB_KSYCOCA)
xml2man_SOURCES = xml2man.cpp

xslt_pure.o: xslt.cpp
	-rm -f xslt_pure.cpp
	$(LN_S) $(srcdir)/xslt.cpp xslt_pure.cpp
	$(CXXCOMPILE) -DSIMPLE_XSLT -c xslt_pure.cpp
	-rm -f xslt_pure.cpp

saxon_LDADD = $(LIB_KSYCOCA) libxslt/libxslt.la
saxon_SOURCES = saxon.cpp xslt.cpp kio_help.cpp

checkXML_SOURCES = checkXML.cpp xslt.cpp kio_help.cpp
checkXML_LDADD = $(LIB_KSYCOCA) libxslt/libxslt.la

bin_SCRIPTS = xmlizer kdb2html

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


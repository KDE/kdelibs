SUBDIRS = libxslt

INCLUDES= $(all_includes) $(LIBXML_CFLAGS)
LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIBXML_RPATH)

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

bin_SCRIPTS = xmlizer

METASOURCES = AUTO

messages:
	$(XGETTEXT) *.cpp -o $(podir)/kio_help.pot

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(kde_datadir)/ksgmltools2
	@echo "****************************************************"
	@echo "** WARNING: I INSTALL LINKS HERE FOR EASIER DEVEL **"
	@echo "** DON'T REMEMBER ME TO INSTALL THE FILES WHEN WE **"
	@echo "** COME NEAR A RELEASE.             THANKS, COOLO **"
	@echo "****************************************************"
	@pwd=`cd $(srcdir) && pwd` ;\
	if false; then \
	rm -rf $(DESTDIR)$(kde_datadir)/ksgmltools2/docbook ;\
	$(LN_S) $$pwd/docbook $(DESTDIR)$(kde_datadir)/ksgmltools2/docbook ;\
	rm -rf $(DESTDIR)$(kde_datadir)/ksgmltools2/customization ;\
	$(LN_S) $$pwd/customization $(DESTDIR)$(kde_datadir)/ksgmltools2/customization;\
	else \
	rm -rf $(DESTDIR)$(kde_datadir)/ksgmltools2/docbook ;\
	cp -r $(srcdir)/docbook $(DESTDIR)$(kde_datadir)/ksgmltools2 ;\
	rm -rf $(DESTDIR)$(kde_datadir)/ksgmltools2/customization ;\
	cp -r $(srcdir)/customization $(DESTDIR)$(kde_datadir)/ksgmltools2 ;\
	fi


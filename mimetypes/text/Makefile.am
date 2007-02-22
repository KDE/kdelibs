
mimetypetextdata_DATA = x-c++src.desktop x-c++hdr.desktop x-csrc.desktop x-java.desktop \
	x-makefile.desktop x-moc.desktop x-pascal.desktop x-objchdr.desktop \
	x-tcl.desktop x-tex.desktop html.desktop plain.desktop enriched.desktop x-chdr.desktop \
	rtf.desktop sgml.desktop xml.desktop css.desktop x-vcard.desktop x-ldif.desktop \
	x-vcalendar.desktop x-diff.desktop x-objcsrc.desktop x-log.desktop \
	x-bibtex.desktop rdf.desktop rss.desktop calendar.desktop x-adasrc.desktop \
	x-csv.desktop x-latex.desktop x-xslfo.desktop x-xslt.desktop vnd.wap.wml.desktop x-katefilelist.desktop \
	docbook.desktop x-mswinurl.desktop x-hex.desktop vnd.abc.desktop javascript.desktop x-python.desktop

mimetypetextdatadir = $(kde_mimedir)/text

install-data-local: uninstall.desktop
	$(mkinstalldirs) $(DESTDIR)$(mimetypetextdatadir)
	$(INSTALL_DATA) $(srcdir)/uninstall.desktop $(DESTDIR)$(mimetypetextdatadir)/x-perl.desktop
	$(INSTALL_DATA) $(srcdir)/uninstall.desktop $(DESTDIR)$(mimetypetextdatadir)/x-python.desktop
	$(INSTALL_DATA) $(srcdir)/uninstall.desktop $(DESTDIR)$(mimetypetextdatadir)/x-lyx.desktop

EXTRA_DIST = $(mimetypetextdata_DATA)

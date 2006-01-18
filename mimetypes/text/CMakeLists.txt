kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${KDE4_MIME_DIR}/text FILES x-c++src.desktop x-c++hdr.desktop x-csrc.desktop x-java.desktop  	x-makefile.desktop x-moc.desktop x-pascal.desktop x-objchdr.desktop  	x-tcl.desktop x-tex.desktop html.desktop plain.desktop enriched.desktop x-chdr.desktop  	rtf.desktop sgml.desktop xml.desktop css.desktop x-vcard.desktop x-ldif.desktop  	x-vcalendar.desktop x-diff.desktop x-objcsrc.desktop x-log.desktop  	x-bibtex.desktop rdf.desktop rss.desktop calendar.desktop x-adasrc.desktop  	x-csv.desktop x-latex.desktop x-xslfo.desktop x-xslt.desktop vnd.wap.wml.desktop x-katefilelist.desktop  	docbook.desktop x-mswinurl.desktop x-hex.desktop vnd.abc.desktop javascript.desktop )

kde4_footer()



#original Makefile.am contents follow:

#
#mimetypetextdata_DATA = x-c++src.desktop x-c++hdr.desktop x-csrc.desktop x-java.desktop \
#	x-makefile.desktop x-moc.desktop x-pascal.desktop x-objchdr.desktop \
#	x-tcl.desktop x-tex.desktop html.desktop plain.desktop enriched.desktop x-chdr.desktop \
#	rtf.desktop sgml.desktop xml.desktop css.desktop x-vcard.desktop x-ldif.desktop \
#	x-vcalendar.desktop x-diff.desktop x-objcsrc.desktop x-log.desktop \
#	x-bibtex.desktop rdf.desktop rss.desktop calendar.desktop x-adasrc.desktop \
#	x-csv.desktop x-latex.desktop x-xslfo.desktop x-xslt.desktop vnd.wap.wml.desktop x-katefilelist.desktop \
#	docbook.desktop x-mswinurl.desktop x-hex.desktop vnd.abc.desktop javascript.desktop
#
#mimetypetextdatadir = $(kde_mimedir)/text
#
#install-data-local: uninstall.desktop
#	$(mkinstalldirs) $(DESTDIR)$(mimetypetextdatadir)
#	$(INSTALL_DATA) $(srcdir)/uninstall.desktop $(DESTDIR)$(mimetypetextdatadir)/x-perl.desktop
#	$(INSTALL_DATA) $(srcdir)/uninstall.desktop $(DESTDIR)$(mimetypetextdatadir)/x-python.desktop
#	$(INSTALL_DATA) $(srcdir)/uninstall.desktop $(DESTDIR)$(mimetypetextdatadir)/x-lyx.desktop
#
#EXTRA_DIST = $(mimetypetextdata_DATA)

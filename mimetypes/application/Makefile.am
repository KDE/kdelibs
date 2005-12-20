
mimetypeapplicationdata_DATA = x-dvi.desktop x-gzdvi.desktop x-bz2dvi.desktop \
	x-tex-gf.desktop x-gzip.desktop x-pak.desktop \
	x-object.desktop x-tex-pk.desktop pdf.desktop \
	x-tar.desktop x-trash.desktop x-zip.desktop \
	x-core.desktop x-javascript.desktop \
	java.desktop x-java.desktop x-java-applet.desktop \
	x-jar.desktop x-arj.desktop x-tarz.desktop \
	x-gzpostscript.desktop postscript.desktop octet-stream.desktop \
	x-shellscript.desktop x-desktop.desktop \
	x-executable.desktop x-executable-script.desktop x-archive.desktop x-lha.desktop \
	x-tbz.desktop x-tgz.desktop x-awk.desktop \
	x-perl.desktop x-perl-module.desktop x-compress.desktop x-cpio.desktop \
	x-rpm.desktop msword.desktop x-sharedlib.desktop \
	wordperfect.desktop x-troff.desktop x-troff-man.desktop \
	x-python.desktop x-python-bytecode.desktop \
	x-bzip.desktop x-bzip2.desktop mspowerpoint.desktop \
	msexcel.desktop smil.desktop x-rar.desktop x-zoo.desktop \
	x-tzo.desktop x-lzop.desktop \
	x-applixword.desktop x-applixspread.desktop x-applixgraphics.desktop \
	xhtml+xml.desktop vnd.ms-excel.desktop vnd.ms-word.desktop \
	x-shockwave-flash.desktop x-designer.desktop x-linguist.desktop \
	x-ogg.desktop ogg.desktop x-pkcs12.desktop x-x509-ca-cert.desktop \
	pkcs7-signature.desktop pgp-signature.desktop pgp-encrypted.desktop \
	pgp-keys.desktop pgp.desktop x-msmetafile.desktop \
	x-mswrite.desktop illustrator.desktop x-abiword.desktop \
	x-gnumeric.desktop x-graphite.desktop x-karbon.desktop x-kchart.desktop \
	x-kformula.desktop x-kivio.desktop x-kontour.desktop \
	x-kpresenter.desktop x-krita.desktop \
	x-kspread.desktop x-kugar.desktop x-kword.desktop \
	x-quattropro.desktop x-amipro.desktop x-tgif.desktop \
	x-lyx.desktop x-zerosize.desktop x-kpovmodeler.desktop x-cda.desktop \
	pkcs7-mime.desktop pkcs10.desktop sieve.desktop mbox.desktop \
	x-hancomword.desktop \
	x-font-bdf.desktop x-font-pcf.desktop x-font-speedo.desktop x-font-type1.desktop \
	x-font-ghostscript.desktop x-font-snf.desktop x-font-ttf.desktop \
	vnd.ms-asf.desktop x-msdos-program.desktop x-deb.desktop \
	vnd.stardivision.writer.desktop vnd.stardivision.calc.desktop \
	vnd.stardivision.impress.desktop \
	vnd.sun.xml.impress.desktop vnd.sun.xml.writer.desktop \
	vnd.sun.xml.base.desktop \
	x-magicpoint.desktop x-vnd.kde.kugar.mixed.desktop x-vnd.kde.kexi.desktop \
	vnd.palm.desktop x-kudesigner.desktop x-vnd.kde.kplato.desktop \
	vnd.sun.xml.calc.desktop mathml+xml.desktop x-bittorrent.desktop \
	x-msaccess.desktop x-gettext.desktop x-ruby.desktop x-kcsrc.desktop \
	x-php.desktop x-font-ttc.desktop x-font-otf.desktop \
	vnd.mozilla.xul+xml.desktop vnd.sun.xml.draw.desktop \
	x-kde-wallet.desktop x-cue.desktop x-ica.desktop x-iso.desktop x-ace.desktop \
	x-afm.desktop x-arc.desktop vnd.rn-realmedia.desktop x-dbase.desktop \
	vnd.sun.xml.calc.template.desktop vnd.sun.xml.writer.template.desktop \
	vnd.sun.xml.impress.template.desktop vnd.sun.xml.draw.template.desktop \
	vnd.stardivision.chart.desktop vnd.stardivision.draw.desktop \
	vnd.stardivision.math.desktop vnd.stardivision.writer-global.desktop vnd.sun.xml.writer.master.desktop \
	x-xliff.desktop x-7z.desktop xml.desktop x-webarchive.desktop  x-mimearchive.desktop \
	xsd.desktop relaxng.desktop \
	vnd.oasis.opendocument.graphics-template.desktop \
	vnd.oasis.opendocument.graphics.desktop \
	vnd.oasis.opendocument.formula.desktop \
	vnd.oasis.opendocument.presentation-template.desktop \
	vnd.oasis.opendocument.presentation.desktop \
	vnd.oasis.opendocument.spreadsheet-template.desktop \
	vnd.oasis.opendocument.spreadsheet.desktop \
	vnd.oasis.opendocument.text-template.desktop \
	vnd.oasis.opendocument.text.desktop \
	vnd.oasis.opendocument.image.desktop \
	vnd.oasis.opendocument.chart.desktop \
	x-java-jnlp-file.desktop xml-dtd.desktop fits.desktop \
	x-rar-compressed.desktop \
	x-sqlite2.desktop x-sqlite3.desktop

mimetypeapplicationdatadir = $(kde_mimedir)/application

install-data-local: uninstall.desktop
	$(mkinstalldirs) $(DESTDIR)$(mimetypeapplicationdatadir)
	$(INSTALL_DATA) $(srcdir)/uninstall.desktop $(DESTDIR)$(mimetypeapplicationdatadir)/x-applix.desktop
	$(INSTALL_DATA) $(srcdir)/uninstall.desktop $(DESTDIR)$(mimetypeapplicationdatadir)/x-oowriter.desktop
	$(INSTALL_DATA) $(srcdir)/uninstall.desktop $(DESTDIR)$(mimetypeapplicationdatadir)/x-ooimpress.desktop

EXTRA_DIST = $(mimetypeapplicationdata_DATA)


data_DATA = x-dvi.desktop x-tex-gf.desktop x-gzip.desktop \
	x-object.desktop x-tex-pk.desktop pdf.desktop \
	x-tar.desktop x-trash.desktop x-zip.desktop \
	x-core.desktop x-java.desktop \
	x-jar.desktop x-arj.desktop x-tarz.desktop \
	postscript.desktop octet-stream.desktop \
	x-shellscript.desktop x-desktop.desktop \
	x-executable.desktop x-archive.desktop x-lha.desktop \
	x-tbz.desktop x-tgz.desktop x-awk.desktop \
	x-perl.desktop x-compress.desktop x-cpio.desktop \
	x-rpm.desktop msword.desktop msword2.desktop x-sharedlib.desktop \
	wordperfect.desktop x-troff.desktop x-troff-man.desktop \
	x-python.desktop x-python-bytecode.desktop \
	x-bzip.desktop x-bzip2.desktop mspowerpoint.desktop \
	msexcel.desktop smil.desktop x-rar.desktop x-zoo.desktop \
	x-tzo.desktop x-lzop.desktop x-mswinurl.desktop \
	x-applixword.desktop x-applixspread.desktop x-applixgraphics.desktop \
	msexcel2.desktop \
	x-shockwave-flash.desktop x-designer.desktop x-linguist.desktop \
	x-ogg.desktop x-pkcs12.desktop x-x509-ca-cert.desktop \
	pkcs7-signature.desktop pgp-signature.desktop pgp-encrypted.desktop \
	x-mswrite.desktop illustrator.desktop x-abiword.desktop \
	x-gnumeric.desktop x-graphite.desktop x-karbon.desktop x-kchart.desktop \
	x-kformula.desktop x-kivio.desktop x-kontour.desktop \
	x-kpresenter.desktop x-krita.desktop x-kspread-crypt.desktop \
	x-kspread.desktop x-kugar.desktop x-kword-crypt.desktop x-kword.desktop \
	x-quattropro.desktop x-amipro.desktop x-tgif.desktop \
	x-lyx.desktop x-zerosize.desktop x-kpovmodeler.desktop x-cda.desktop \
	pkcs7-mime.desktop pkcs10.desktop sieve.desktop \
	x-hancomword.desktop \
	x-font-bdf.desktop x-font-pcf.desktop x-font-speedo.desktop x-font-type1.desktop \
	x-font-ghostscript.desktop x-font-snf.desktop x-font-ttf.desktop

datadir = $(kde_mimedir)/application

install-data-local: uninstall.desktop
	$(mkinstalldirs) $(DESTDIR)$(datadir)
	$(INSTALL_DATA) $(srcdir)/uninstall.desktop $(DESTDIR)$(datadir)/x-applix.desktop

EXTRA_DIST = $(data_DATA)


data_DATA = jpeg.desktop tiff.desktop x-xpm.desktop x-xbm.desktop svg+xml.desktop \
	png.desktop gif.desktop fax-g3.desktop cgm.desktop x-bmp.desktop \
	x-photo-cd.desktop x-eps.desktop x-ico.desktop x-jng.desktop x-xcf-gimp.desktop \
	x-wmf.desktop

datadir = $(kde_mimedir)/image

install-data-local: uninstall.desktop
	$(mkinstalldirs) $(datadir)
	$(INSTALL_DATA) $(srcdir)/uninstall.desktop $(datadir)/wmf.desktop

EXTRA_DIST = $(data_DATA)



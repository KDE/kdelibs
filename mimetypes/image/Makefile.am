
mimetypeimagedata_DATA = jpeg.desktop jpg.desktop tiff.desktop x-xpm.desktop x-xbm.desktop \
    svg+xml.desktop svg-xml.desktop png.desktop gif.desktop fax-g3.desktop cgm.desktop \
    x-bmp.desktop x-photo-cd.desktop x-eps.desktop x-ico.desktop \
    x-jng.desktop x-xcf-gimp.desktop x-wmf.desktop x-msod.desktop \
    x-xfig.desktop x-portable-bitmap.desktop x-portable-greymap.desktop \
    x-portable-pixmap.desktop x-pcx.desktop x-targa.desktop jp2.desktop \
    x-vnd.trolltech.qpicture.desktop x-vnd.adobe.photoshop.desktop \
    x-xcursor.desktop pjpeg.desktop x-exr.desktop \
    x-rgb.desktop x-dds.desktop x-djvu.desktop x-hdr.desktop fits.desktop \
    x-raw.desktop

mimetypeimagedatadir = $(kde_mimedir)/image

install-data-local: uninstall.desktop
	$(mkinstalldirs) $(DESTDIR)$(mimetypeimagedatadir)
	$(INSTALL_DATA) $(srcdir)/uninstall.desktop $(DESTDIR)$(mimetypeimagedatadir)/wmf.desktop

EXTRA_DIST = $(mimetypeimagedata_DATA)

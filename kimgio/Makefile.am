lib_LTLIBRARIES= kimg_eps.la kimg_xview.la kimg_krl.la \
	         $(TIFF_MODULE)

if HAVE_LIBTIFF
TIFF_MODULE = kimg_tiff.la
TIFF_KIMGIO = tiff.kimgio
endif

kimg_tiff_la_SOURCES = tiffr.cpp
kimg_tiff_la_LDFLAGS = -module -avoid-version $(USER_LDFLAGS)
kimg_tiff_la_LIBADD = $(LIBTIFF) 

kimg_xview_la_SOURCES = xview.cpp
kimg_xview_la_LDFLAGS = -module -avoid-version $(USER_LDFLAGS)
kimg_xview_la_LIBADD = 

kimg_krl_la_SOURCES = krl.cpp
kimg_krl_la_LDFLAGS = -module -avoid-version $(USER_LDFLAGS)
kimg_krl_la_LIBADD = 

kimg_eps_la_SOURCES = eps.cpp
kimg_eps_la_LDFLAGS = -module -avoid-version $(USER_LDFLAGS)
kimg_eps_la_LIBADD = 

noinst_HEADERS= xview.h eps.h tiffr.h krl.h

INCLUDES = -I$(top_srcdir)/kio -I$(top_srcdir)/dcop -I$(top_srcdir)/libltdl $(all_includes)

EXTRA_DIST = netpbm.cpp netpbm.h

servicedir = $(kde_servicesdir)
service_DATA = png.kimgio xpm.kimgio bmp.kimgio gif.kimgio pnm.kimgio xbm.kimgio \
               jpeg.kimgio xv.kimgio krl.kimgio eps.kimgio \
               $(TIFF_KIMGIO)



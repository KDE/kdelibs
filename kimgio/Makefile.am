
lib_LTLIBRARIES= libkimgio.la kimg_eps.la kimg_xview.la \
	$(JPEG_MODULE) $(PNG_MODULE) $(TIFF_MODULE)

if HAVE_LIBJPEG
JPEG_MODULE = kimg_jpeg.la
endif

if HAVE_LIBPNG
PNG_MODULE = kimg_png.la
endif

if HAVE_LIBTIFF
TIFF_MODULE = kimg_tiff.la
endif

kimg_jpeg_la_SOURCES = jpeg.cpp
kimg_jpeg_la_LDFLAGS = -module
kimg_jpeg_la_LIBADD = $(LIBJPEG)

kimg_png_la_SOURCES = pngr.cpp
kimg_png_la_LDFLAGS = -module
kimg_png_la_LIBADD = $(LIBPNG)

kimg_tiff_la_SOURCES = tiffr.cpp
kimg_tiff_la_LDFLAGS = -module
kimg_tiff_la_LIBADD = $(LIBTIFF)

kimg_xview_la_SOURCES = xview.cpp
kimg_xview_la_LDFLAGS = -module

kimg_eps_la_SOURCES = eps.cpp
kimg_eps_la_LDFLAGS = -module

libkimgio_la_LDFLAGS = -version-info 3:0 --export-dynamic 
libkimgio_la_LIBADD = ../libltdl/libltdlc.la 
libkimgio_la_SOURCES = kimgio.cpp 
include_HEADERS = kimgio.h

noinst_HEADERS= jpeg.h xview.h eps.h tiffr.h pngr.h

INCLUDES = -I$(top_srcdir)/libltdl $(all_includes)

EXTRA_DIST = netpbm.cpp netpbm.h

check_PROGRAMS = test
test_SOURCES = test.cpp
test_LDFLAGS = $(all_libraries)
test_LDADD = $(LIB_QT) libkimgio.la ../kdecore/libkdecore.la

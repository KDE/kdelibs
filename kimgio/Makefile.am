
if include_TIFF_MODULES
KIMGIO_TIFF_MODULES=kimg_tiff.la # kimg_g3.la
KIMGIO_TIFF_DATAFILES=tiff.kimgio # g3.kimgio
endif

if include_JP2_MODULES
KIMGIO_JP2_MODULES=kimg_jp2.la
KIMGIO_JP2_DATAFILES=jp2.kimgio
endif

if include_EXR_MODULES
KIMGIO_EXR_MODULES=kimg_exr.la
KIMGIO_EXR_DATAFILES=exr.kimgio
endif

KDE_CXXFLAGS = $(USE_EXCEPTIONS)

# kimg_hdr.la missing and JP2_MODULES
kde_imageformats_LTLIBRARIES= kimg_eps.la kimg_xview.la  \
	$(KIMGIO_TIFF_MODULES) kimg_ico.la \
	kimg_pcx.la kimg_tga.la kimg_rgb.la kimg_xcf.la kimg_dds.la  \
	$(KIMGIO_EXR_MODULES) kimg_psd.la 

KIMGIO_PLUGIN = $(KDE_PLUGIN) -module $(all_libraries)

kimg_tiff_la_SOURCES = tiffr.cpp
kimg_tiff_la_LDFLAGS = $(KIMGIO_PLUGIN)
kimg_tiff_la_LIBADD = $(LIBTIFF) $(LIB_QT)

#kimg_g3_la_SOURCES = g3r.cpp
#kimg_g3_la_LDFLAGS = -module $(USER_LDFLAGS) $(KIMGIO_PLUGIN) -no-undefined
#kimg_g3_la_LIBADD = $(LIBTIFF)   

kimg_xview_la_SOURCES = xview.cpp
kimg_xview_la_LDFLAGS = $(KIMGIO_PLUGIN)
kimg_xview_la_LIBADD  = $(LIB_QT)

kimg_eps_la_SOURCES = eps.cpp
kimg_eps_la_LDFLAGS = $(KIMGIO_PLUGIN)
kimg_eps_la_LIBADD  = $(LIB_KDECORE)

kimg_ico_la_SOURCES = ico.cpp
kimg_ico_la_LDFLAGS = $(KIMGIO_PLUGIN)
kimg_ico_la_LIBADD  = $(LIB_QT)

kimg_jp2_la_SOURCES = jp2.cpp
kimg_jp2_la_LDFLAGS = $(KIMGIO_PLUGIN)
kimg_jp2_la_LIBADD = $(LIB_QT) $(LIB_JASPER) $(LIB_KDECORE)

kimg_pcx_la_SOURCES = pcx.cpp
kimg_pcx_la_LDFLAGS = $(KIMGIO_PLUGIN)
kimg_pcx_la_LIBADD = $(LIB_QT) $(LIB_KDECORE)

kimg_tga_la_SOURCES = tga.cpp
kimg_tga_la_LDFLAGS = $(KIMGIO_PLUGIN)
kimg_tga_la_LIBADD = $(LIB_QT) $(LIB_KDECORE)

kimg_rgb_la_SOURCES = rgb.cpp
kimg_rgb_la_LDFLAGS = $(KIMGIO_PLUGIN) 
kimg_rgb_la_LIBADD = $(LIB_QT) $(LIB_KDECORE)

kimg_xcf_la_SOURCES = xcf.cpp
kimg_xcf_la_LDFLAGS = $(KIMGIO_PLUGIN)
kimg_xcf_la_LIBADD = $(LIB_QT) $(LIB_KDECORE)

kimg_exr_la_SOURCES = exr.cpp
kimg_exr_la_LDFLAGS = $(KIMGIO_PLUGIN)
kimg_exr_la_LIBADD = $(LIB_QT) $(LIB_KDECORE) $(LIB_EXR)

kimg_dds_la_SOURCES = dds.cpp
kimg_dds_la_LDFLAGS = $(KIMGIO_PLUGIN)
kimg_dds_la_LIBADD = $(LIB_QT) $(LIB_KDECORE)

kimg_psd_la_SOURCES = psd.cpp
kimg_psd_la_LDFLAGS = $(KIMGIO_PLUGIN)
kimg_psd_la_LIBADD = $(LIB_QT) $(LIB_KDECORE)

kimg_hdr_la_SOURCES = hdr.cpp
kimg_hdr_la_LDFLAGS = -module $(KIMGIO_PLUGIN) -no-undefined $(all_libraries)
kimg_hdr_la_LIBADD = $(LIB_QT) $(LIB_KDECORE)

noinst_HEADERS= xview.h eps.h tiffr.h g3r.h ico.h jp2.h pcx.h tga.h rgb.h \
	xcf.h gimp.h exr.h dds.h psd.h hdr.h

INCLUDES = -I$(top_srcdir)/kio -I$(top_srcdir)/dcop -I$(top_srcdir)/libltdl \
	$(all_includes) -Drestrict= $(EXR_FLAGS)

servicedir = $(kde_servicesdir)
service_DATA = png.kimgio xpm.kimgio bmp.kimgio pbm.kimgio pgm.kimgio \
              ppm.kimgio xbm.kimgio jpeg.kimgio xv.kimgio eps.kimgio \
           $(KIMGIO_TIFF_DATAFILES) ico.kimgio $(KIMGIO_JP2_DATAFILES) \
           gif.kimgio pcx.kimgio tga.kimgio rgb.kimgio xcf.kimgio dds.kimgio \
           $(KIMGIO_EXR_DATAFILES) mng.kimgio psd.kimgio hdr.kimgio

include $(top_srcdir)/admin/Doxyfile.am


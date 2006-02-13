kde4_header()

macro_optional_find_package(Jasper)
macro_optional_find_package(TIFF)
macro_optional_find_package(OpenEXR)
include_directories( ${CMAKE_SOURCE_DIR}/kio ${CMAKE_SOURCE_DIR}/dcop ${CMAKE_SOURCE_DIR}/libltdl ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )

if(JASPER_FOUND)
	set(kimg_jp2_LIB_SRCS jp2.cpp)
	kde4_add_plugin(kimg_jp2 ${kimg_jp2_LIB_SRCS})
	target_link_libraries(kimg_jp2  ${QT_AND_KDECORE_LIBS} ${JASPER_LIBRARIES})
	
	kde4_install_libtool_file(${KDE4_PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_jp2 )
	install_targets(${KDE4_PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_jp2 )
	install_files( ${KDE4_SERVICES_DIR}/qimageioplugins/ FILES jp2.desktop)
endif(JASPER_FOUND)		

##################################
set(kimg_ico_LIB_SRCS ico.cpp)
kde4_add_plugin(kimg_ico ${kimg_ico_LIB_SRCS})
target_link_libraries(kimg_ico  ${QT_AND_KDECORE_LIBS})

kde4_install_libtool_file(${KDE4_PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_ico )
install_targets(${KDE4_PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_ico )

##################################
set(kimg_eps_LIB_SRCS eps.cpp)
kde4_add_plugin(kimg_eps ${kimg_eps_LIB_SRCS})
target_link_libraries(kimg_eps  ${QT_AND_KDECORE_LIBS})

kde4_install_libtool_file(${KDE4_PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_eps )
install_targets(${KDE4_PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_eps )

##################################
set(kimg_pcx_LIB_SRCS pcx.cpp)
kde4_add_plugin(kimg_pcx ${kimg_pcx_LIB_SRCS})
target_link_libraries(kimg_pcx  ${QT_AND_KDECORE_LIBS})

kde4_install_libtool_file(${KDE4_PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_pcx )
install_targets(${KDE4_PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_pcx )

##################################
set(kimg_tga_LIB_SRCS tga.cpp)
kde4_add_plugin(kimg_tga ${kimg_tga_LIB_SRCS})
target_link_libraries(kimg_tga  ${QT_AND_KDECORE_LIBS})

kde4_install_libtool_file(${KDE4_PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_tga )
install_targets(${KDE4_PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_tga )

##################################
set(kimg_rgb_LIB_SRCS rgb.cpp)
kde4_add_plugin(kimg_rgb ${kimg_rgb_LIB_SRCS})
target_link_libraries(kimg_rgb  ${QT_AND_KDECORE_LIBS})

kde4_install_libtool_file(${KDE4_PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_rgb )
install_targets(${KDE4_PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_rgb )

##################################
set(kimg_xcf_LIB_SRCS xcf.cpp)
kde4_add_plugin(kimg_xcf ${kimg_xcf_LIB_SRCS})
target_link_libraries(kimg_xcf ${QT_AND_KDECORE_LIBS})

kde4_install_libtool_file(${KDE4_PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_xcf )
install_targets(${KDE4_PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_xcf )


##################################
set(kimg_dds_LIB_SRCS dds.cpp)
kde4_add_plugin(kimg_dds ${kimg_dds_LIB_SRCS})
target_link_libraries(kimg_dds ${QT_AND_KDECORE_LIBS})

kde4_install_libtool_file(${KDE4_PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_dds )
install_targets(${KDE4_PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_dds )


##################################
set(kimg_psd_LIB_SRCS psd.cpp)
kde4_add_plugin(kimg_psd ${kimg_psd_LIB_SRCS})
target_link_libraries(kimg_psd ${QT_AND_KDECORE_LIBS})

kde4_install_libtool_file(${KDE4_PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_psd )
install_targets(${KDE4_PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_psd )

##################################                                              
set(kimg_xview_LIB_SRCS xview.cpp)
kde4_add_plugin(kimg_xview ${kimg_xview_LIB_SRCS})
target_link_libraries(kimg_xview ${QT_AND_KDECORE_LIBS})
kde4_install_libtool_file(${KDE4_PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_xview )  
install_targets(${KDE4_PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_xview )

##################################
if(OPENEXR_FOUND)
	set(kimg_exr_LIB_SRCS exr.cpp)
	kde4_add_plugin(kimg_exr ${kimg_exr_LIB_SRCS})
	target_link_libraries(kimg_exr ${QT_AND_KDECORE_LIBS} )

	kde4_install_libtool_file(${KDE4_PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_exr )
	install_targets(${KDE4_PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_exr )	
    install_files( ${KDE4_SERVICES_DIR}/qimageioplugins/ FILES exr.desktop)
endif(OPENEXR_FOUND)

if(TIFF_FOUND)
    set(kimg_tiff_LIB_SRCS tiffr.cpp)
    kde4_add_plugin(kimg_tiff ${kimg_tiff_LIB_SRCS})
    target_link_libraries(kimg_tiff ${QT_AND_KDECORE_LIBS} ${TIFF_LIBRARY})

    kde4_install_libtool_file(${KDE4_PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_tiff )
    install_targets(${KDE4_PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_tiff )
    install_files( ${KDE4_SERVICES_DIR}/qimageioplugins/ FILES tiff.desktop)
endif(TIFF_FOUND)



########### install files ###############

# FIX : $(KIMGIO_TIFF_SERVICEFILES) 
install_files( ${KDE4_SERVICES_DIR}/qimageioplugins/ FILES png.desktop xpm.desktop bmp.desktop pbm.desktop pgm.desktop  ppm.desktop xbm.desktop jpeg.desktop xv.desktop eps.desktop ico.desktop gif.desktop pcx.desktop tga.desktop rgb.desktop xcf.desktop dds.desktop mng.desktop psd.desktop hdr.desktop )




install_files( ${KDE4_SERVICETYPES_DIR}/ FILES qimageio_plugin.desktop)

kde4_create_doxygen_docs(  )

kde4_footer()



#original Makefile.am contents follow:

#
#if include_TIFF_MODULES
#KIMGIO_TIFF_MODULES=kimg_tiff.la # kimg_g3.la
#KIMGIO_TIFF_DATAFILES=tiff.kimgio # g3.kimgio
#endif
#
#if include_JP2_MODULES
#KIMGIO_JP2_MODULES=kimg_jp2.la
#KIMGIO_JP2_DATAFILES=jp2.kimgio
#endif
#
#if include_EXR_MODULES
#KIMGIO_EXR_MODULES=kimg_exr.la
#KIMGIO_EXR_DATAFILES=exr.kimgio
#endif
#
#KDE_CXXFLAGS = $(USE_EXCEPTIONS)
#
## kimg_hdr.la missing and JP2_MODULES
#kde_imageformats_LTLIBRARIES= kimg_eps.la kimg_xview.la  \
#	$(KIMGIO_TIFF_MODULES) kimg_ico.la \
#	kimg_pcx.la kimg_tga.la kimg_rgb.la kimg_xcf.la kimg_dds.la  \
#	$(KIMGIO_EXR_MODULES) kimg_psd.la 
#
#KIMGIO_PLUGIN = $(KDE_PLUGIN) -module $(all_libraries)
#
#kimg_tiff_la_SOURCES = tiffr.cpp
#kimg_tiff_la_LDFLAGS = $(KIMGIO_PLUGIN)
#kimg_tiff_la_LIBADD = $(LIBTIFF) $(LIB_QT)
#
##kimg_g3_la_SOURCES = g3r.cpp
##kimg_g3_la_LDFLAGS = -module $(USER_LDFLAGS) $(KIMGIO_PLUGIN) -no-undefined
##kimg_g3_la_LIBADD = $(LIBTIFF)   
#
#kimg_xview_la_SOURCES = xview.cpp
#kimg_xview_la_LDFLAGS = $(KIMGIO_PLUGIN)
#kimg_xview_la_LIBADD  = $(LIB_QT)
#
#kimg_eps_la_SOURCES = eps.cpp
#kimg_eps_la_LDFLAGS = $(KIMGIO_PLUGIN)
#kimg_eps_la_LIBADD  = $(LIB_KDECORE)
#
#kimg_ico_la_SOURCES = ico.cpp
#kimg_ico_la_LDFLAGS = $(KIMGIO_PLUGIN)
#kimg_ico_la_LIBADD  = $(LIB_QT)
#
#kimg_jp2_la_SOURCES = jp2.cpp
#kimg_jp2_la_LDFLAGS = $(KIMGIO_PLUGIN)
#kimg_jp2_la_LIBADD = $(LIB_QT) $(LIB_JASPER) $(LIB_KDECORE)
#
#kimg_pcx_la_SOURCES = pcx.cpp
#kimg_pcx_la_LDFLAGS = $(KIMGIO_PLUGIN)
#kimg_pcx_la_LIBADD = $(LIB_QT) $(LIB_KDECORE)
#
#kimg_tga_la_SOURCES = tga.cpp
#kimg_tga_la_LDFLAGS = $(KIMGIO_PLUGIN)
#kimg_tga_la_LIBADD = $(LIB_QT) $(LIB_KDECORE)
#
#kimg_rgb_la_SOURCES = rgb.cpp
#kimg_rgb_la_LDFLAGS = $(KIMGIO_PLUGIN) 
#kimg_rgb_la_LIBADD = $(LIB_QT) $(LIB_KDECORE)
#
#kimg_xcf_la_SOURCES = xcf.cpp
#kimg_xcf_la_LDFLAGS = $(KIMGIO_PLUGIN)
#kimg_xcf_la_LIBADD = $(LIB_QT) $(LIB_KDECORE)
#
#kimg_exr_la_SOURCES = exr.cpp
#kimg_exr_la_LDFLAGS = $(KIMGIO_PLUGIN)
#kimg_exr_la_LIBADD = $(LIB_QT) $(LIB_KDECORE) $(LIB_EXR)
#
#kimg_dds_la_SOURCES = dds.cpp
#kimg_dds_la_LDFLAGS = $(KIMGIO_PLUGIN)
#kimg_dds_la_LIBADD = $(LIB_QT) $(LIB_KDECORE)
#
#kimg_psd_la_SOURCES = psd.cpp
#kimg_psd_la_LDFLAGS = $(KIMGIO_PLUGIN)
#kimg_psd_la_LIBADD = $(LIB_QT) $(LIB_KDECORE)
#
#kimg_hdr_la_SOURCES = hdr.cpp
#kimg_hdr_la_LDFLAGS = -module $(KIMGIO_PLUGIN) -no-undefined $(all_libraries)
#kimg_hdr_la_LIBADD = $(LIB_QT) $(LIB_KDECORE)
#
#noinst_HEADERS= xview.h eps.h tiffr.h g3r.h ico.h jp2.h pcx.h tga.h rgb.h \
#	xcf.h gimp.h exr.h dds.h psd.h hdr.h
#
#INCLUDES = -I$(top_srcdir)/kio -I$(top_srcdir)/dcop -I$(top_srcdir)/libltdl \
#	$(all_includes) -Drestrict= $(EXR_FLAGS)
#
#servicedir = $(kde_servicesdir)
#service_DATA = png.kimgio xpm.kimgio bmp.kimgio pbm.kimgio pgm.kimgio \
#              ppm.kimgio xbm.kimgio jpeg.kimgio xv.kimgio eps.kimgio \
#           $(KIMGIO_TIFF_DATAFILES) ico.kimgio $(KIMGIO_JP2_DATAFILES) \
#           gif.kimgio pcx.kimgio tga.kimgio rgb.kimgio xcf.kimgio dds.kimgio \
#           $(KIMGIO_EXR_DATAFILES) mng.kimgio psd.kimgio hdr.kimgio
#
#include $(top_srcdir)/admin/Doxyfile.am
#

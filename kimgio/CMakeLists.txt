
project(kimgio)

macro_optional_find_package(Jasper)
macro_bool_to_01(JASPER_FOUND HAVE_JASPER 1)

macro_optional_find_package(TIFF)
macro_bool_to_01(TIFF_FOUND HAVE_LIBTIFF 1)

macro_optional_find_package(OpenEXR)
macro_bool_to_01(OPENEXR_FOUND HAVE_EXR 1)

configure_file(config-kimgio.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kimgio.h )

include_directories( ${KDE4_KIO_INCLUDES} )

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")


if(JASPER_FOUND)

    set(kimg_jp2_LIB_SRCS jp2.cpp)
    kde4_add_plugin(kimg_jp2 ${kimg_jp2_LIB_SRCS})
    target_link_libraries(kimg_jp2  ${KDE4_KDECORE_LIBS} ${JASPER_LIBRARIES} )
    
    #kde4_install_libtool_file(${PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_jp2 )
    install_targets(${PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_jp2 )
    install_files( ${SERVICES_INSTALL_DIR}/qimageioplugins/ FILES jp2.desktop)

endif(JASPER_FOUND)

##################################

set(kimg_ico_LIB_SRCS ico.cpp)
kde4_add_plugin(kimg_ico ${kimg_ico_LIB_SRCS})
target_link_libraries(kimg_ico  ${KDE4_KDECORE_LIBS})

#kde4_install_libtool_file(${PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_ico )
install_targets(${PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_ico )

##################################

set(kimg_eps_LIB_SRCS eps.cpp)
kde4_add_plugin(kimg_eps ${kimg_eps_LIB_SRCS})
target_link_libraries(kimg_eps  ${KDE4_KDECORE_LIBS})

#kde4_install_libtool_file(${PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_eps )
install_targets(${PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_eps )

##################################

set(kimg_pcx_LIB_SRCS pcx.cpp)
kde4_add_plugin(kimg_pcx ${kimg_pcx_LIB_SRCS})
target_link_libraries(kimg_pcx  ${KDE4_KDECORE_LIBS})

#kde4_install_libtool_file(${PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_pcx )
install_targets(${PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_pcx )

##################################

set(kimg_tga_LIB_SRCS tga.cpp)
kde4_add_plugin(kimg_tga ${kimg_tga_LIB_SRCS})
target_link_libraries(kimg_tga  ${KDE4_KDECORE_LIBS})

#kde4_install_libtool_file(${PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_tga )
install_targets(${PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_tga )

##################################

set(kimg_rgb_LIB_SRCS rgb.cpp)
kde4_add_plugin(kimg_rgb ${kimg_rgb_LIB_SRCS})
target_link_libraries(kimg_rgb  ${KDE4_KDECORE_LIBS})

#kde4_install_libtool_file(${PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_rgb )
install_targets(${PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_rgb )

##################################

set(kimg_xcf_LIB_SRCS xcf.cpp)
kde4_add_plugin(kimg_xcf ${kimg_xcf_LIB_SRCS})
target_link_libraries(kimg_xcf ${KDE4_KDECORE_LIBS})

#kde4_install_libtool_file(${PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_xcf )
install_targets(${PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_xcf )


##################################

set(kimg_dds_LIB_SRCS dds.cpp)
kde4_add_plugin(kimg_dds ${kimg_dds_LIB_SRCS})
target_link_libraries(kimg_dds ${KDE4_KDECORE_LIBS})

#kde4_install_libtool_file(${PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_dds )
install_targets(${PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_dds )


##################################

set(kimg_psd_LIB_SRCS psd.cpp)
kde4_add_plugin(kimg_psd ${kimg_psd_LIB_SRCS})
target_link_libraries(kimg_psd ${KDE4_KDECORE_LIBS})

#kde4_install_libtool_file(${PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_psd )
install_targets(${PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_psd )

##################################
                                              
set(kimg_xview_LIB_SRCS xview.cpp)
kde4_add_plugin(kimg_xview ${kimg_xview_LIB_SRCS})
target_link_libraries(kimg_xview ${KDE4_KDECORE_LIBS})

#kde4_install_libtool_file(${PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_xview )  
install_targets(${PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_xview )

##################################

if(OPENEXR_FOUND)
    include_directories( ${CMAKE_SOURCE_DIR}/kio ${CMAKE_SOURCE_DIR}/dcop ${CMAKE_SOURCE_DIR}/libltdl ${OPENEXR_INCLUDE_DIR} )
    set(kimg_exr_LIB_SRCS exr.cpp)
    kde4_add_plugin(kimg_exr ${kimg_exr_LIB_SRCS})
    target_link_libraries(kimg_exr ${KDE4_KDECORE_LIBS} ${OPENEXR_LIBRARIES} )

    #kde4_install_libtool_file(${PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_exr )
    install_targets(${PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_exr )
    install_files( ${SERVICES_INSTALL_DIR}/qimageioplugins/ FILES exr.desktop)
endif(OPENEXR_FOUND)


if(TIFF_FOUND)
    include_directories( ${TIFF_INCLUDE_DIR} )
    set(kimg_tiff_LIB_SRCS tiffr.cpp)
    kde4_add_plugin(kimg_tiff ${kimg_tiff_LIB_SRCS})
    target_link_libraries(kimg_tiff ${KDE4_KDECORE_LIBS} ${TIFF_LIBRARY})

    #kde4_install_libtool_file(${PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_tiff )
    install_targets(${PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_tiff )
    install_files( ${SERVICES_INSTALL_DIR}/qimageioplugins/ FILES tiff.desktop)
endif(TIFF_FOUND)



########### install files ###############

install_files( ${SERVICES_INSTALL_DIR}/qimageioplugins/ FILES png.desktop xpm.desktop bmp.desktop pbm.desktop pgm.desktop  ppm.desktop xbm.desktop jpeg.desktop xv.desktop eps.desktop ico.desktop gif.desktop pcx.desktop tga.desktop rgb.desktop xcf.desktop dds.desktop mng.desktop psd.desktop hdr.desktop )

install_files( ${SERVICETYPES_INSTALL_DIR}/ FILES qimageio_plugin.desktop)





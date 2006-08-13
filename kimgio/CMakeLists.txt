
project(kimgio)

macro_optional_find_package(Jasper)
macro_bool_to_01(JASPER_FOUND HAVE_JASPER)

macro_optional_find_package(TIFF)
macro_bool_to_01(TIFF_FOUND HAVE_LIBTIFF)

macro_optional_find_package(OpenEXR)
macro_bool_to_01(OPENEXR_FOUND HAVE_EXR)

configure_file(config-kimgio.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kimgio.h )

include_directories( ${KDE4_KIO_INCLUDES} )

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")


if(JASPER_FOUND)

    include_directories( ${JASPER_INCLUDE_DIR} )
    set(kimg_jp2_LIB_SRCS jp2.cpp)
    kde4_add_plugin(kimg_jp2 ${kimg_jp2_LIB_SRCS})
    target_link_libraries(kimg_jp2  ${KDE4_KDECORE_LIBS} ${JASPER_LIBRARIES} )
    
    #kde4_install_libtool_file(${PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_jp2 )
    install(TARGETS kimg_jp2  DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/imageformats/ )
    install( FILES jp2.desktop DESTINATION  ${SERVICES_INSTALL_DIR}/qimageioplugins/ )

endif(JASPER_FOUND)

##################################

set(kimg_ico_LIB_SRCS ico.cpp)
kde4_add_plugin(kimg_ico ${kimg_ico_LIB_SRCS})
target_link_libraries(kimg_ico  ${KDE4_KDECORE_LIBS})

#kde4_install_libtool_file(${PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_ico )
install(TARGETS kimg_ico  DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/imageformats/ )

##################################

set(kimg_eps_LIB_SRCS eps.cpp)
kde4_add_plugin(kimg_eps ${kimg_eps_LIB_SRCS})
target_link_libraries(kimg_eps  ${KDE4_KDECORE_LIBS})

#kde4_install_libtool_file(${PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_eps )
install(TARGETS kimg_eps  DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/imageformats/ )

##################################

set(kimg_pcx_LIB_SRCS pcx.cpp)
kde4_add_plugin(kimg_pcx ${kimg_pcx_LIB_SRCS})
target_link_libraries(kimg_pcx  ${KDE4_KDECORE_LIBS})

#kde4_install_libtool_file(${PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_pcx )
install(TARGETS kimg_pcx  DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/imageformats/ )

##################################

set(kimg_tga_LIB_SRCS tga.cpp)
kde4_add_plugin(kimg_tga ${kimg_tga_LIB_SRCS})
target_link_libraries(kimg_tga  ${KDE4_KDECORE_LIBS})

#kde4_install_libtool_file(${PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_tga )
install(TARGETS kimg_tga  DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/imageformats/ )

##################################

set(kimg_rgb_LIB_SRCS rgb.cpp)
kde4_add_plugin(kimg_rgb ${kimg_rgb_LIB_SRCS})
target_link_libraries(kimg_rgb  ${KDE4_KDECORE_LIBS})

#kde4_install_libtool_file(${PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_rgb )
install(TARGETS kimg_rgb  DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/imageformats/ )

##################################

set(kimg_xcf_LIB_SRCS xcf.cpp)
kde4_add_plugin(kimg_xcf ${kimg_xcf_LIB_SRCS})
target_link_libraries(kimg_xcf ${KDE4_KDECORE_LIBS})

#kde4_install_libtool_file(${PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_xcf )
install(TARGETS kimg_xcf  DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/imageformats/ )


##################################

set(kimg_dds_LIB_SRCS dds.cpp)
kde4_add_plugin(kimg_dds ${kimg_dds_LIB_SRCS})
target_link_libraries(kimg_dds ${KDE4_KDECORE_LIBS})

#kde4_install_libtool_file(${PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_dds )
install(TARGETS kimg_dds  DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/imageformats/ )


##################################

set(kimg_psd_LIB_SRCS psd.cpp)
kde4_add_plugin(kimg_psd ${kimg_psd_LIB_SRCS})
target_link_libraries(kimg_psd ${KDE4_KDECORE_LIBS})

#kde4_install_libtool_file(${PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_psd )
install(TARGETS kimg_psd  DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/imageformats/ )

##################################
                                              
set(kimg_xview_LIB_SRCS xview.cpp)
kde4_add_plugin(kimg_xview ${kimg_xview_LIB_SRCS})
target_link_libraries(kimg_xview ${KDE4_KDECORE_LIBS})

#kde4_install_libtool_file(${PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_xview )  
install(TARGETS kimg_xview  DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/imageformats/ )

##################################

if(OPENEXR_FOUND)
    include_directories( ${CMAKE_SOURCE_DIR}/kio ${CMAKE_SOURCE_DIR}/dcop ${CMAKE_SOURCE_DIR}/libltdl ${OPENEXR_INCLUDE_DIR} )
    set(kimg_exr_LIB_SRCS exr.cpp)
    kde4_add_plugin(kimg_exr ${kimg_exr_LIB_SRCS})
    target_link_libraries(kimg_exr ${KDE4_KDECORE_LIBS} ${OPENEXR_LIBRARIES} )

    #kde4_install_libtool_file(${PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_exr )
install(TARGETS kimg_exr  DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/imageformats/ )
install( FILES exr.desktop DESTINATION  ${SERVICES_INSTALL_DIR}/qimageioplugins/ )
endif(OPENEXR_FOUND)


if(TIFF_FOUND AND NOT MSVC)
    include_directories( ${TIFF_INCLUDE_DIR} )
    set(kimg_tiff_LIB_SRCS tiffr.cpp)
    kde4_add_plugin(kimg_tiff ${kimg_tiff_LIB_SRCS})
    target_link_libraries(kimg_tiff ${KDE4_KDECORE_LIBS} ${TIFF_LIBRARY})

    #kde4_install_libtool_file(${PLUGIN_INSTALL_DIR}/plugins/imageformats/ kimg_tiff )
install(TARGETS kimg_tiff  DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/imageformats/ )
install( FILES tiff.desktop DESTINATION  ${SERVICES_INSTALL_DIR}/qimageioplugins/ )
endif(TIFF_FOUND AND NOT MSVC)



########### install files ###############

install( FILES png.desktop xpm.desktop bmp.desktop pbm.desktop pgm.desktop  ppm.desktop xbm.desktop jpeg.desktop xv.desktop eps.desktop ico.desktop gif.desktop pcx.desktop tga.desktop rgb.desktop xcf.desktop dds.desktop mng.desktop psd.desktop hdr.desktop  DESTINATION  ${SERVICES_INSTALL_DIR}/qimageioplugins/ )

install( FILES qimageio_plugin.desktop DESTINATION  ${SERVICETYPES_INSTALL_DIR}/ )





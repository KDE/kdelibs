
project(kimgio)

macro_optional_find_package(Jasper)

macro_optional_find_package(OpenEXR)

include_directories( ${KDE4_KIO_INCLUDES} )

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")


if(JASPER_FOUND)

    include_directories( ${JASPER_INCLUDE_DIR} )
    set(kimg_jp2_LIB_SRCS jp2.cpp)
    kde4_add_plugin(kimg_jp2 ${kimg_jp2_LIB_SRCS})
    target_link_libraries(kimg_jp2  ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} ${JASPER_LIBRARIES} )
    
    install(TARGETS kimg_jp2  DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/imageformats/ )
    install( FILES jp2.desktop DESTINATION  ${SERVICES_INSTALL_DIR}/qimageioplugins/ )

endif(JASPER_FOUND)
macro_log_feature(JASPER_FOUND "JasPer" "JasPer JPEG2000 Library" "http://www.ece.uvic.ca/~mdadams/jasper" FALSE "" "Provides support for JPEG-2000 images.")

##################################

set(kimg_eps_LIB_SRCS eps.cpp)
kde4_add_plugin(kimg_eps ${kimg_eps_LIB_SRCS})
target_link_libraries(kimg_eps  ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY})

install(TARGETS kimg_eps  DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/imageformats/ )

##################################

set(kimg_pcx_LIB_SRCS pcx.cpp)
kde4_add_plugin(kimg_pcx ${kimg_pcx_LIB_SRCS})
target_link_libraries(kimg_pcx  ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY})

install(TARGETS kimg_pcx  DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/imageformats/ )

##################################

set(kimg_tga_LIB_SRCS tga.cpp)
kde4_add_plugin(kimg_tga ${kimg_tga_LIB_SRCS})
target_link_libraries(kimg_tga  ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY})

install(TARGETS kimg_tga  DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/imageformats/ )

##################################

set(kimg_ras_LIB_SRCS ras.cpp)
kde4_add_plugin(kimg_ras ${kimg_ras_LIB_SRCS})
target_link_libraries(kimg_ras  ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY})

install(TARGETS kimg_ras  DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/imageformats/ )

##################################

set(kimg_rgb_LIB_SRCS rgb.cpp)
kde4_add_plugin(kimg_rgb ${kimg_rgb_LIB_SRCS})
target_link_libraries(kimg_rgb  ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY})

install(TARGETS kimg_rgb  DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/imageformats/ )

##################################

set(kimg_xcf_LIB_SRCS xcf.cpp)
kde4_add_plugin(kimg_xcf ${kimg_xcf_LIB_SRCS})
target_link_libraries(kimg_xcf ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY})

install(TARGETS kimg_xcf  DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/imageformats/ )


##################################

set(kimg_dds_LIB_SRCS dds.cpp)
kde4_add_plugin(kimg_dds ${kimg_dds_LIB_SRCS})
target_link_libraries(kimg_dds ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY})

install(TARGETS kimg_dds  DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/imageformats/ )


##################################

set(kimg_psd_LIB_SRCS psd.cpp)
kde4_add_plugin(kimg_psd ${kimg_psd_LIB_SRCS})
target_link_libraries(kimg_psd ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY})

install(TARGETS kimg_psd  DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/imageformats/ )

##################################
                                              
set(kimg_xview_LIB_SRCS xview.cpp)
kde4_add_plugin(kimg_xview ${kimg_xview_LIB_SRCS})
target_link_libraries(kimg_xview ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY})

install(TARGETS kimg_xview  DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/imageformats/ )

##################################

if(OPENEXR_FOUND)
    include_directories( ${CMAKE_SOURCE_DIR}/kio ${OPENEXR_INCLUDE_DIR} )
    add_definitions(${OPENEXR_DEFINITIONS})
    set(kimg_exr_LIB_SRCS exr.cpp)
    kde4_add_plugin(kimg_exr ${kimg_exr_LIB_SRCS})
    target_link_libraries(kimg_exr ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} ${OPENEXR_LIBRARIES} )

install(TARGETS kimg_exr  DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/imageformats/ )
install( FILES exr.desktop DESTINATION  ${SERVICES_INSTALL_DIR}/qimageioplugins/ )
endif(OPENEXR_FOUND)
macro_log_feature(OPENEXR_FOUND "OpenEXR" "A library for manipulating OpenEXR image files" "http://www.openexr.com" FALSE "" "Provides support for OpenEXR images.")

##################################
                                              
set(kimg_pic_LIB_SRCS pic_read.cpp pic_write.cpp pic_io_plugin.cpp pic_io_handler.cpp)
kde4_add_plugin(kimg_pic ${kimg_pic_LIB_SRCS})
target_link_libraries(kimg_pic ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY})

install(TARGETS kimg_pic  DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/imageformats/ )


########### install files ###############

install( FILES
  png.desktop
  xpm.desktop
  bmp.desktop
  pbm.desktop
  pgm.desktop
  pnm.desktop
  ppm.desktop
  xbm.desktop
  jpeg.desktop
  xv.desktop
  eps.desktop
  ico.desktop
  gif.desktop
  pcx.desktop
  tga.desktop
  ras.desktop
  rgb.desktop
  xcf.desktop
  dds.desktop
  mng.desktop
  psd.desktop
  hdr.desktop
  tiff.desktop
  pic.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}/qimageioplugins/ )

install( FILES qimageio_plugin.desktop DESTINATION  ${SERVICETYPES_INSTALL_DIR}/ )



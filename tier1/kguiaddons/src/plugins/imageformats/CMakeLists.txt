
find_package(Jasper)
set_package_properties(Jasper PROPERTIES DESCRIPTION "Support for JPEG-2000 images"
                       URL "http://www.ece.uvic.ca/~mdadams/jasper"
                       TYPE OPTIONAL
                      )

find_package(OpenEXR)
set_package_properties(OpenEXR PROPERTIES DESCRIPTION "Support for OpenEXR images"
                       URL "http://www.openexr.com"
                       TYPE OPTIONAL
                      )

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

configure_file(config-kimgio.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kimgio.h )
check_include_files(sys/types.h   HAVE_SYS_TYPES_H)
check_include_files(stdint.h      HAVE_STDINT_H)

if(JASPER_FOUND)

    include_directories( ${JASPER_INCLUDE_DIR} )
    set(kimg_jp2_LIB_SRCS jp2.cpp)
    add_library(kimg_jp2 MODULE ${kimg_jp2_LIB_SRCS})
    set_target_properties(kimg_jp2 PROPERTIES PREFIX "")
    target_link_libraries(kimg_jp2  Qt5::Gui ${JASPER_LIBRARIES} )

    install(TARGETS kimg_jp2  DESTINATION ${QT_PLUGIN_INSTALL_DIR}/imageformats/ )
    install(FILES jp2.desktop DESTINATION  ${SERVICES_INSTALL_DIR}/qimageioplugins/ )

endif()

##################################

# EPS support depends on the gs utility; non-UNIX systems are unlikely to have
# this available in PATH
if (UNIX)
   find_package(Qt5PrintSupport 5.2.0 NO_MODULE)
   set_package_properties(Qt5PrintSupport PROPERTIES
      DESCRIPTION "Support for Qt printing, necessary for the EPS format"
      TYPE OPTIONAL
   )

   if (Qt5PrintSupport_FOUND)
      set(kimg_eps_LIB_SRCS eps.cpp)
      add_library(kimg_eps MODULE ${kimg_eps_LIB_SRCS})
      set_target_properties(kimg_eps PROPERTIES PREFIX "")
      target_link_libraries(kimg_eps  Qt5::Gui Qt5::PrintSupport)

      install(TARGETS kimg_eps  DESTINATION ${QT_PLUGIN_INSTALL_DIR}/imageformats/)
      install(FILES eps.desktop DESTINATION  ${SERVICES_INSTALL_DIR}/qimageioplugins/)
   endif()
endif()

##################################

set(kimg_pcx_LIB_SRCS pcx.cpp)
add_library(kimg_pcx MODULE ${kimg_pcx_LIB_SRCS})
set_target_properties(kimg_pcx PROPERTIES PREFIX "")
target_link_libraries(kimg_pcx  Qt5::Gui)

install(TARGETS kimg_pcx  DESTINATION ${QT_PLUGIN_INSTALL_DIR}/imageformats/ )
install(FILES pcx.desktop DESTINATION  ${SERVICES_INSTALL_DIR}/qimageioplugins/ )

##################################

set(kimg_tga_LIB_SRCS tga.cpp)
add_library(kimg_tga MODULE ${kimg_tga_LIB_SRCS})
set_target_properties(kimg_tga PROPERTIES PREFIX "")
target_link_libraries(kimg_tga  Qt5::Gui)

install(TARGETS kimg_tga  DESTINATION ${QT_PLUGIN_INSTALL_DIR}/imageformats/ )
install(FILES tga.desktop DESTINATION  ${SERVICES_INSTALL_DIR}/qimageioplugins/ )

##################################

set(kimg_ras_LIB_SRCS ras.cpp)
add_library(kimg_ras MODULE ${kimg_ras_LIB_SRCS})
set_target_properties(kimg_ras PROPERTIES PREFIX "")
target_link_libraries(kimg_ras  Qt5::Gui)

install(TARGETS kimg_ras  DESTINATION ${QT_PLUGIN_INSTALL_DIR}/imageformats/ )
install(FILES ras.desktop DESTINATION  ${SERVICES_INSTALL_DIR}/qimageioplugins/ )

##################################

set(kimg_rgb_LIB_SRCS rgb.cpp)
add_library(kimg_rgb MODULE ${kimg_rgb_LIB_SRCS})
set_target_properties(kimg_rgb PROPERTIES PREFIX "")
target_link_libraries(kimg_rgb  Qt5::Gui)

install(TARGETS kimg_rgb  DESTINATION ${QT_PLUGIN_INSTALL_DIR}/imageformats/ )
install(FILES rgb.desktop DESTINATION  ${SERVICES_INSTALL_DIR}/qimageioplugins/ )

##################################

set(kimg_xcf_LIB_SRCS xcf.cpp)
add_library(kimg_xcf MODULE ${kimg_xcf_LIB_SRCS})
set_target_properties(kimg_xcf PROPERTIES PREFIX "")
target_link_libraries(kimg_xcf Qt5::Gui)

install(TARGETS kimg_xcf  DESTINATION ${QT_PLUGIN_INSTALL_DIR}/imageformats/ )
install(FILES xcf.desktop DESTINATION  ${SERVICES_INSTALL_DIR}/qimageioplugins/ )

##################################

set(kimg_dds_LIB_SRCS dds.cpp)
add_library(kimg_dds MODULE ${kimg_dds_LIB_SRCS})
set_target_properties(kimg_dds PROPERTIES PREFIX "")
target_link_libraries(kimg_dds Qt5::Gui)

install(TARGETS kimg_dds  DESTINATION ${QT_PLUGIN_INSTALL_DIR}/imageformats/ )
install(FILES dds.desktop DESTINATION  ${SERVICES_INSTALL_DIR}/qimageioplugins/ )

##################################

set(kimg_psd_LIB_SRCS psd.cpp)
add_library(kimg_psd MODULE ${kimg_psd_LIB_SRCS})
set_target_properties(kimg_psd PROPERTIES PREFIX "")
target_link_libraries(kimg_psd Qt5::Gui)

install(TARGETS kimg_psd  DESTINATION ${QT_PLUGIN_INSTALL_DIR}/imageformats/ )
install(FILES psd.desktop DESTINATION  ${SERVICES_INSTALL_DIR}/qimageioplugins/ )

##################################

set(kimg_xview_LIB_SRCS xview.cpp)
add_library(kimg_xview MODULE ${kimg_xview_LIB_SRCS})
set_target_properties(kimg_xview PROPERTIES PREFIX "")
target_link_libraries(kimg_xview Qt5::Gui)

install(TARGETS kimg_xview  DESTINATION ${QT_PLUGIN_INSTALL_DIR}/imageformats/ )
install(FILES xv.desktop DESTINATION  ${SERVICES_INSTALL_DIR}/qimageioplugins/ )

##################################

if(OPENEXR_FOUND)

    include_directories( ${OPENEXR_INCLUDE_DIR} )
    add_definitions(${OPENEXR_DEFINITIONS})
    set(kimg_exr_LIB_SRCS exr.cpp)
    add_library(kimg_exr MODULE ${kimg_exr_LIB_SRCS})
    set_target_properties(kimg_exr PROPERTIES PREFIX "")
    target_link_libraries(kimg_exr Qt5::Gui ${OPENEXR_LIBRARIES} )

    install(TARGETS kimg_exr  DESTINATION ${QT_PLUGIN_INSTALL_DIR}/imageformats/ )
    install(FILES exr.desktop DESTINATION  ${SERVICES_INSTALL_DIR}/qimageioplugins/ )

endif()

##################################

set(kimg_pic_LIB_SRCS pic_read.cpp pic_write.cpp pic.cpp)
add_library(kimg_pic MODULE ${kimg_pic_LIB_SRCS})
set_target_properties(kimg_pic PROPERTIES PREFIX "")
target_link_libraries(kimg_pic Qt5::Gui)

install(TARGETS kimg_pic  DESTINATION ${QT_PLUGIN_INSTALL_DIR}/imageformats/ )
install(FILES pic.desktop DESTINATION  ${SERVICES_INSTALL_DIR}/qimageioplugins/ )

########### install files ###############

# For plugins supplied with Qt
install( FILES
  png.desktop
  xpm.desktop
  bmp.desktop
  pbm.desktop
  pgm.desktop
  ppm.desktop
  xbm.desktop
  jpeg.desktop
  ico.desktop
  gif.desktop
  mng.desktop
  tiff.desktop
  svg+xml.desktop
  svg+xml-compressed.desktop
  wbmp.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}/qimageioplugins/ )

install( FILES qimageio_plugin.desktop DESTINATION  ${SERVICETYPES_INSTALL_DIR}/ )



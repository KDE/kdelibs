kde4_header()

include_directories( ${CMAKE_SOURCE_DIR} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )

########### next target ###############

set(kdehwifaces_LIB_SRCS 
   devicemanager.cpp 
   device.cpp 
   capability.cpp 
   processor.cpp 
   block.cpp 
   storage.cpp 
   cdrom.cpp 
   volume.cpp 
   opticaldisc.cpp 
   camera.cpp 
   portablemediaplayer.cpp 
   networkiface.cpp 
   acadapter.cpp 
   battery.cpp 
   button.cpp
   display.cpp 
   audioiface.cpp 
   abstractcapability.cpp )

kde4_automoc(${kdehwifaces_LIB_SRCS})

kde4_add_library(kdehwifaces SHARED ${kdehwifaces_LIB_SRCS})

target_link_libraries(kdehwifaces  ${KDE4_KDECORE_LIBS} )

set_target_properties(kdehwifaces PROPERTIES VERSION 4.2.0 SOVERSION 4 )
install_targets(${LIB_INSTALL_DIR} kdehwifaces )


########### install files ###############

install_files( /include/kdehw/ifaces FILES  devicemanager.h device.h enums.h capability.h processor.h block.h storage.h cdrom.h volume.h opticaldisc.h camera.h portablemediaplayer.h networkiface.h acadapter.h battery.h button.h display.h audioiface.h abstractcapability.h )

kde4_footer()

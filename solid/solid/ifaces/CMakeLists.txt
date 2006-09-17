

include_directories( ${CMAKE_SOURCE_DIR} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )

########### next target ###############

set(solidifaces_LIB_SRCS 
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
   abstractcapability.cpp
   powermanager.cpp 
   networkdevice.cpp
   network.cpp
   wirelessnetwork.cpp
   authentication.cpp
   networkmanager.cpp )

kde4_automoc(${solidifaces_LIB_SRCS})

kde4_add_library(solidifaces SHARED ${solidifaces_LIB_SRCS})

target_link_libraries(solidifaces  ${KDE4_KDECORE_LIBS} )

set_target_properties(solidifaces PROPERTIES VERSION 4.2.0 SOVERSION 4 )
install(TARGETS solidifaces DESTINATION ${LIB_INSTALL_DIR}  )


########### install files ###############

install( FILES devicemanager.h device.h enums.h capability.h processor.h block.h storage.h cdrom.h volume.h opticaldisc.h camera.h portablemediaplayer.h networkiface.h acadapter.h battery.h button.h display.h audioiface.h abstractcapability.h powermanager.h networkdevice.h network.h wirelessnetwork.h authentication.h DESTINATION ${INCLUDE_INSTALL_DIR}/solid/ifaces)



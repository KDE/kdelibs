

add_subdirectory( tests )

########### next target ###############

set(solid_hal_discovery_PART_SRCS 
   halmanager.cpp 
   haldevice.cpp 
   halcalljob.cpp 
   deviceinterface.cpp 
   genericinterface.cpp 
   processor.cpp 
   block.cpp 
   storage.cpp 
   cdrom.cpp 
   volume.cpp 
   opticaldisc.cpp 
   camera.cpp 
   portablemediaplayer.cpp 
   networkhw.cpp 
   acadapter.cpp 
   battery.cpp 
   button.cpp 
   display.cpp
   audiohw.cpp
   dvbhw.cpp )

kde4_automoc(${solid_hal_discovery_PART_SRCS})

kde4_add_library(solid_hal_discovery_static STATIC ${solid_hal_discovery_PART_SRCS})
target_link_libraries(solid_hal_discovery_static ${KDE4_KDECORE_LIBS} solidifaces )

kde4_add_plugin(solid_hal_discovery module_discovery.cpp ${solid_hal_discovery_PART_SRCS})

target_link_libraries(solid_hal_discovery ${KDE4_KDECORE_LIBS} solidifaces )



install(TARGETS solid_hal_discovery DESTINATION ${PLUGIN_INSTALL_DIR}  )


########### install files ###############

install( FILES solid_hal_discovery.desktop DESTINATION ${SERVICES_INSTALL_DIR}/solidbackends )



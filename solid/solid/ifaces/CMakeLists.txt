

########### next target ###############

set(solidifaces_LIB_SRCS 
   devicemanager.cpp 
   device.cpp 
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
   dvbhw.cpp
)

kde4_automoc(${solidifaces_LIB_SRCS})

kde4_add_library(solidifaces_static STATIC ${solidifaces_LIB_SRCS})

target_link_libraries(solidifaces_static  ${KDE4_KDECORE_LIBS} )

#set_target_properties(solidifaces PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
#install(TARGETS solidifaces DESTINATION ${LIB_INSTALL_DIR}  )


########### install files ###############

#install( FILES devicemanager.h device.h deviceinterface.h genericinterface.h processor.h block.h storage.h cdrom.h volume.h opticaldisc.h camera.h portablemediaplayer.h networkhw.h acadapter.h battery.h button.h display.h audiohw.h dvbhw.h DESTINATION ${INCLUDE_INSTALL_DIR}/solid/ifaces)



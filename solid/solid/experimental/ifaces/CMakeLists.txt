

########### next target ###############

set(solidexperimentalifaces_LIB_SRCS 
   bluetoothremotedevice.cpp
   bluetoothinputdevice.cpp
   bluetoothinterface.cpp
   bluetoothmanager.cpp )

kde4_automoc(${solidexperimentalifaces_LIB_SRCS})

kde4_add_library(solidexperimentalifaces SHARED ${solidexperimentalifaces_LIB_SRCS})

target_link_libraries(solidexperimentalifaces  ${KDE4_KDECORE_LIBS} )

set_target_properties(solidexperimentalifaces PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS solidexperimentalifaces DESTINATION ${LIB_INSTALL_DIR}  )


########### install files ###############

install( FILES bluetoothremotedevice.h bluetoothinputdevice.h bluetoothinterface.h bluetoothmanager.h DESTINATION ${INCLUDE_INSTALL_DIR}/solid/experimental/ifaces)


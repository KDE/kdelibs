add_subdirectory( ifaces )

########### next target ###############

set(solidexperimental_LIB_SRCS 
   frontendobject.cpp
   managerbase.cpp
   powermanager.cpp
   networkmanager.cpp
   networkinterface.cpp
   network.cpp
   wirelessnetwork.cpp
   authentication.cpp
   bluetoothinterface.cpp
   bluetoothmanager.cpp
   bluetoothremotedevice.cpp
   bluetoothinputdevice.cpp
)

kde4_automoc(${solidexperimental_LIB_SRCS})

kde4_add_library(solidexperimental SHARED ${solidexperimental_LIB_SRCS})

target_link_libraries(solidexperimental ${KDE4_KDECORE_LIBS} solidexperimentalifaces )

set_target_properties(solidexperimental PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS solidexperimental DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############

install( FILES solidpowermanager.desktop solidnetworkmanager.desktop solidbluetoothmanager.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR}  )
install( FILES powermanager.h networkmanager.h networkinterface.h network.h wirelessnetwork.h authentication.h bluetoothmanager.h bluetoothinterface.h bluetoothremotedevice.h bluetoothinputdevice.h singletondefs.h DESTINATION ${INCLUDE_INSTALL_DIR}/solid/experimental)


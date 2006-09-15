add_subdirectory( ifaces )

include_directories( ${CMAKE_SOURCE_DIR} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )

set( solidnet_LIB_SRCS
    device.cpp
    network.cpp
    wirelessnetwork.cpp
	authentication.cpp
)

kde4_automoc( ${solidnet_LIB_SRCS} )

kde4_add_library( solidnet SHARED ${solidnet_LIB_SRCS} )

target_link_libraries( solidnet ${KDE4_KDECORE_LIBS} solidnetifaces )

set_target_properties(solidnet PROPERTIES VERSION 4.2.0 SOVERSION 4 )
install_targets(${LIB_INSTALL_DIR} solidnet )

install( FILES device.h network.h wirelessnetwork.h DESTINATION include/solid/net/ifaces )

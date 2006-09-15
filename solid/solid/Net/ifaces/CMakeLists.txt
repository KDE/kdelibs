

include_directories( ${CMAKE_SOURCE_DIR} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )

########### next target ###############

set(solidnetifaces_LIB_SRCS
   device.cpp
   network.cpp
   wirelessnetwork.cpp
   authentication.cpp
)

kde4_automoc(${solidnetifaces_LIB_SRCS})

kde4_add_library(solidnetifaces SHARED ${solidnetifaces_LIB_SRCS})

target_link_libraries(solidnetifaces  ${KDE4_KDECORE_LIBS} )

set_target_properties(solidnetifaces PROPERTIES VERSION 4.2.0 SOVERSION 4 )
install_targets(${LIB_INSTALL_DIR} solidnetifaces )


########### install files ###############

install( FILES device.h enums.h network.h wirelessnetwork.h DESTINATION include/solid/net/ifaces )



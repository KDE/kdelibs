
include_directories( ${CMAKE_SOURCE_DIR}/kded )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")
#remove_definitions(-DQT3_SUPPORT)

########### next target ###############

set(kded_proxyscout_PART_SRCS
   proxyscout.cpp
   script.cpp
   downloader.cpp
   discovery.cpp
)

kde4_automoc(${kded_proxyscout_PART_SRCS})

kde4_add_plugin(kded_proxyscout ${kded_proxyscout_PART_SRCS})

target_link_libraries(kded_proxyscout  ${KDE4_KDECORE_LIBS} kio kjs )

# this needs -lresolv e.g on Slackware, but not on FreeBSD
if (HAVE_RESOLV_LIBRARY)
   target_link_libraries(kded_proxyscout resolv)
endif (HAVE_RESOLV_LIBRARY)

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kded_proxyscout )
install_targets(${PLUGIN_INSTALL_DIR} kded_proxyscout )


########### next target ###############

set(kpac_dhcp_helper_SRCS kpac_dhcp_helper.c )

kde4_add_executable(kpac_dhcp_helper NOGUI ${kpac_dhcp_helper_SRCS})

target_link_libraries(kpac_dhcp_helper  ${KDE4_KDECORE_LIBS} )

install(TARGETS kpac_dhcp_helper DESTINATION bin)


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/proxyscout FILES proxyscout.notifyrc )
install_files( ${SERVICES_INSTALL_DIR}/kded FILES proxyscout.desktop )





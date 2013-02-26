
if(KDE_PLATFORM_FEATURE_BINARY_COMPATIBLE_FEATURE_REDUCTION)
    set(KPAC_NO_SOLID TRUE)
endif(KDE_PLATFORM_FEATURE_BINARY_COMPATIBLE_FEATURE_REDUCTION)

include_directories( ${CMAKE_SOURCE_DIR}/kded )

if(NOT KPAC_NO_SOLID)
    include_directories(${Solid_INCLUDE_DIRS})
   set(KPAC_EXTRA_LIBS ${Solid_LIBRARIES})
endif(NOT KPAC_NO_SOLID)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

include(ConfigureChecks.cmake)
configure_file(config-kpac.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kpac.h)

########### next target ###############

if (Qt5Script_FOUND)
    set(kded_proxyscout_PART_SRCS
      proxyscout.cpp
      script.cpp
      downloader.cpp
      discovery.cpp
    )

    kde4_add_plugin(kded_proxyscout ${kded_proxyscout_PART_SRCS})

    target_link_libraries(kded_proxyscout ${QT_QTSCRIPT_LIBRARY} ${KPAC_EXTRA_LIBS} ${KDE4_KDECORE_LIBS} kio ${KCoreAddons_LIBRARIES} )

    # this needs -lresolv e.g on Slackware, but not on FreeBSD
    if (HAVE_RESOLV_LIBRARY)
      target_link_libraries(kded_proxyscout resolv)
    endif (HAVE_RESOLV_LIBRARY)

    install(TARGETS kded_proxyscout  DESTINATION ${PLUGIN_INSTALL_DIR} )
endif()

########### next target ###############

set(kpac_dhcp_helper_SRCS kpac_dhcp_helper.c)

kde4_add_executable(kpac_dhcp_helper NOGUI ${kpac_dhcp_helper_SRCS})

if (HAVE_NSL_LIBRARY)
  # Assume Solaris
  target_link_libraries(kpac_dhcp_helper nsl)
endif (HAVE_NSL_LIBRARY)

if (HAVE_SOCKET_LIBRARY)
  target_link_libraries(kpac_dhcp_helper socket)
endif (HAVE_SOCKET_LIBRARY)

install(TARGETS kpac_dhcp_helper DESTINATION ${LIBEXEC_INSTALL_DIR} )

########### install files ###############

install( FILES proxyscout.notifyrc  DESTINATION  ${DATA_INSTALL_DIR}/proxyscout )
install( FILES proxyscout.desktop  DESTINATION  ${SERVICES_INSTALL_DIR}/kded )


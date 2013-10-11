
if(KDE_PLATFORM_FEATURE_BINARY_COMPATIBLE_FEATURE_REDUCTION)
    set(KPAC_NO_SOLID TRUE)
endif(KDE_PLATFORM_FEATURE_BINARY_COMPATIBLE_FEATURE_REDUCTION)

if(NOT KPAC_NO_SOLID)
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

    add_library(kded_proxyscout MODULE ${kded_proxyscout_PART_SRCS})

    target_link_libraries(kded_proxyscout Qt5::Script ${KPAC_EXTRA_LIBS} kio KF5::KCoreAddons )

    # this needs -lresolv e.g on Slackware, but not on FreeBSD
    if (HAVE_RESOLV_LIBRARY)
      target_link_libraries(kded_proxyscout resolv)
    endif (HAVE_RESOLV_LIBRARY)

    install(TARGETS kded_proxyscout  DESTINATION ${PLUGIN_INSTALL_DIR} )
endif()

########### next target ###############

add_executable(kpac_dhcp_helper kpac_dhcp_helper.c)
ecm_mark_nongui_executable(kpac_dhcp_helper)

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


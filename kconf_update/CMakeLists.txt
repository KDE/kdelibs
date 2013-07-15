
project(kconf_update)

configure_file(config-kconf.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kconf.h )

########### next target ###############

set(kconf_update_KDEINIT_SRCS
    kconf_update.cpp
    kconfigutils.cpp
    )


kde4_add_kdeinit_executable( kconf_update NOGUI ${kconf_update_KDEINIT_SRCS})

target_include_directories(kdeinit_kconf_update PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/..)

target_link_libraries(kdeinit_kconf_update kdeqt5staging ${KCoreAddons_LIBRARIES} KI18n  ${KConfig_CORE_LIBRARY})

install(TARGETS kdeinit_kconf_update ${INSTALL_TARGETS_DEFAULT_ARGS})

target_link_libraries( kconf_update kdeinit_kconf_update )
install(TARGETS kconf_update DESTINATION ${LIBEXEC_INSTALL_DIR} )

########### install files ###############

add_subdirectory(tests)





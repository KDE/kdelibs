
project(kconf_update)

configure_file(config-kconf.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kconf.h )

########### next target ###############

set(kconf_update_SRCS
    kconf_update.cpp
    kconfigutils.cpp
    )

add_executable(kconf_update ${kconf_update_SRCS})
target_link_libraries(kconf_update kdeqt5staging ${KCoreAddons_LIBRARIES} ${KConfig_CORE_LIBRARY})

install(TARGETS kconf_update DESTINATION ${LIBEXEC_INSTALL_DIR})


add_subdirectory(tests)


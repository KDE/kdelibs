find_package(Qt5Core 5.2.0 REQUIRED NO_MODULE)

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

configure_file(config-kconf.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kconf.h )

########### next target ###############

set(kconf_update_SRCS
    kconf_update.cpp
    kconfigutils.cpp
    )

add_executable(kconf_update ${kconf_update_SRCS})
target_link_libraries(kconf_update Qt5::Core kdeqt5staging KConfigCore)

install(TARGETS kconf_update DESTINATION ${LIBEXEC_INSTALL_DIR})

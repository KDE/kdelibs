
project(kconf_update)

include_directories( ${KDE4_KDECORE_INCLUDES} )


########### next target ###############

set(kconf_update_KDEINIT_SRCS
    kconf_update.cpp
    kconfigutils.cpp
    )


kde4_add_kdeinit_executable( kconf_update NOGUI ${kconf_update_KDEINIT_SRCS})

target_link_libraries(kdeinit_kconf_update  ${KDE4_KDECORE_LIBS} )

install(TARGETS kdeinit_kconf_update ${INSTALL_TARGETS_DEFAULT_ARGS})

target_link_libraries( kconf_update kdeinit_kconf_update )
install(TARGETS kconf_update DESTINATION ${LIBEXEC_INSTALL_DIR} )

########### install files ###############

add_subdirectory(tests)





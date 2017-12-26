
project(kconf_update)

include_directories( ${KDE4_KDECORE_INCLUDES} )


########### next target ###############

set(kconf_update_KDEINIT_SRCS
    kconf_update.cpp
    kconfigutils.cpp
    )

if (WIN32)
    # set uiaccess to false in manifest
    file(READ ${CMAKE_SOURCE_DIR}/cmake/modules/Win32.Manifest.in _tmp)
    string(REPLACE "true" "false" _out ${_tmp})
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/Win32.Manifest ${_out})
    # create rc file
    # see https://stackoverflow.com/questions/33000158/embed-manifest-file-to-require-administrator-execution-level-with-mingw32
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/file.rc "1 24 \"${CMAKE_CURRENT_BINARY_DIR}/Win32.Manifest\"\n")
    list(APPEND kconf_update_KDEINIT_SRCS ${CMAKE_CURRENT_BINARY_DIR}/file.rc)
endif()

kde4_add_kdeinit_executable( kconf_update NOGUI ${kconf_update_KDEINIT_SRCS})

target_link_libraries(kdeinit_kconf_update  ${KDE4_KDECORE_LIBS} )

install(TARGETS kdeinit_kconf_update ${INSTALL_TARGETS_DEFAULT_ARGS})

target_link_libraries( kconf_update kdeinit_kconf_update )
install(TARGETS kconf_update DESTINATION ${LIBEXEC_INSTALL_DIR} )

########### install files ###############

add_subdirectory(tests)





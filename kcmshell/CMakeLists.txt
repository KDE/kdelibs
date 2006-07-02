
project(kcmshell)

include_directories( ${KDE4_KPARTS_INCLUDES} )
include_directories( ${CMAKE_SOURCE_DIR}/kutils )
add_definitions( ${QDBUS_DEFINITIONS} )


########### next target ###############

set(kcmshell_KDEINIT_SRCS main.cpp )

kde4_automoc(${kcmshell_KDEINIT_SRCS})

kde4_add_kdeinit_executable( kcmshell ${kcmshell_KDEINIT_SRCS})

target_link_libraries(kdeinit_kcmshell  ${KDE4_KDECORE_LIBS} kutils )

install_targets(${LIB_INSTALL_DIR} kdeinit_kcmshell )

target_link_libraries( kcmshell kdeinit_kcmshell )
install(TARGETS kcmshell DESTINATION bin)

########### install files ###############






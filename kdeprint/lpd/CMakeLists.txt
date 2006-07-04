
include_directories( ${CMAKE_SOURCE_DIR}/kdeprint/management )


########### next target ###############

set(kdeprint_lpd_PART_SRCS
   kmlpdfactory.cpp
   kmlpdmanager.cpp
   klpdprinterimpl.cpp
   kmlpduimanager.cpp
   lpdtools.cpp
   gschecker.cpp
)

kde4_automoc(${kdeprint_lpd_PART_SRCS})

kde4_add_plugin(kdeprint_lpd ${kdeprint_lpd_PART_SRCS})

target_link_libraries(kdeprint_lpd  ${KDE4_KDECORE_LIBS} kdeprint_management )

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kdeprint_lpd )
install(TARGETS kdeprint_lpd  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### next target ###############

set(make_driver_db_lpd_SRCS
make_driver_db_lpd.c
)

if (UNIX)
   set(KDEFAKES_LIB kdefakes)
endif (UNIX)

kde4_automoc(${make_driver_db_lpd_SRCS})

kde4_add_executable(make_driver_db_lpd ${make_driver_db_lpd_SRCS})

target_link_libraries(make_driver_db_lpd  ${KDE4_KDECORE_LIBS} ${KDEFAKES_LIB} )

install(TARGETS make_driver_db_lpd DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############

install( FILES lpd.print  DESTINATION  ${DATA_INSTALL_DIR}/kdeprint/plugins )






add_subdirectory( cupsdconf2 )

include_directories( ${CUPS_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/kdeprint/management )


########### next target ###############

set(kdeprint_cups_PART_SRCS
cupsinfos.cpp
ipprequest.cpp
kmcupsfactory.cpp
kmcupsjobmanager.cpp
kmcupsmanager.cpp
kmcupsuimanager.cpp
kmpropbanners.cpp
kmwbanners.cpp
kmwipp.cpp
kmwippselect.cpp
kcupsprinterimpl.cpp
kmcupsconfigwidget.cpp
kmconfigcups.cpp
kmconfigcupsdir.cpp
kmwippprinter.cpp
kmwfax.cpp
kmwquota.cpp
kmpropquota.cpp
kmwusers.cpp
kmpropusers.cpp
kpschedulepage.cpp
image.cpp
imagepreview.cpp
imageposition.cpp
kpimagepage.cpp
kptextpage.cpp
kphpgl2page.cpp
cupsaddsmb2.cpp
kmwother.cpp
ippreportdlg.cpp
kptagspage.cpp
)

if (UNIX)
   set(KDEFAKES_LIB kdefakes)
endif (UNIX)

kde4_automoc(${kdeprint_cups_PART_SRCS})

kde4_add_plugin(kdeprint_cups ${kdeprint_cups_PART_SRCS})

target_link_libraries(kdeprint_cups  ${KDE4_KDECORE_LIBS} kdeprint_management kio ${KDEFAKES_LIB} ${CUPS_LIBRARIES} )

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kdeprint_cups )
install_targets(${PLUGIN_INSTALL_DIR} kdeprint_cups )


########### next target ###############

kde4_add_executable(make_driver_db_cups NOGUI make_driver_db_cups.c ${CMAKE_CURRENT_SOURCE_DIR}/../driverparse.c )

target_link_libraries(make_driver_db_cups ${KDE4_KDECORE_LIBS} ${KDEFAKES_LIB} )

install(TARGETS make_driver_db_cups DESTINATION bin)


########### next target ###############

kde4_add_executable(cupsdoprint cupsdoprint.c)

target_link_libraries(cupsdoprint  ${KDE4_KDECORE_LIBS} ${KDEFAKES_LIB} ${CUPS_LIBRARIES})

install(TARGETS cupsdoprint DESTINATION bin)


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kdeprint FILES preview.png preview-mini.png )
install_files( ${DATA_INSTALL_DIR}/kdeprint/plugins FILES cups.print )





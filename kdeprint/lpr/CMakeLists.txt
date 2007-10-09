
include_directories( ${CMAKE_SOURCE_DIR}/kdeprint/management )


########### next target ###############

set(kdeprint_lpr_PART_SRCS
printcapreader.cpp
lprhandler.cpp
matichandler.cpp
klprfactory.cpp
kmlprmanager.cpp
kmlpruimanager.cpp
lpchelper.cpp
lpqhelper.cpp
kmlprjobmanager.cpp
matichelper.cpp
klprprinterimpl.cpp
printcapentry.cpp
lprsettings.cpp
kmconfiglpr.cpp
apshandler.cpp
lprngtoolhandler.cpp
editentrydialog.cpp
)


kde4_add_plugin(kdeprint_lpr ${kdeprint_lpr_PART_SRCS})

target_link_libraries(kdeprint_lpr  ${KDE4_KDECORE_LIBS} kdeprint_management ${KDEFAKES_LIB} )

install(TARGETS kdeprint_lpr  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### next target ###############

# windows TODO
if (UNIX)

   kde4_add_executable(make_driver_db_lpr NOGUI make_driver_db_lpr.c ${CMAKE_CURRENT_SOURCE_DIR}/../driverparse.c )

   target_link_libraries(make_driver_db_lpr  ${KDE4_KDECORE_LIBS} ${KDEFAKES_LIB} )

   install(TARGETS make_driver_db_lpr DESTINATION ${LIBEXEC_INSTALL_DIR} )

endif (UNIX)

########### install files ###############

install( FILES apsdriver1 apsdriver2 lprngtooldriver1  DESTINATION  ${DATA_INSTALL_DIR}/kdeprint )
install( FILES lpr.print  DESTINATION  ${DATA_INSTALL_DIR}/kdeprint/plugins )

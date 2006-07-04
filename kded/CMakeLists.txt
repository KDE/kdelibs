
project(kded)


include_directories( ${KDE4_KIO_INCLUDES} )

########### next target ###############

set(kded_KDEINIT_SRCS kded.cpp )

kde4_automoc(${kded_KDEINIT_SRCS})

kde4_add_kdeinit_executable( kded ${kded_KDEINIT_SRCS})

target_link_libraries(kdeinit_kded  ${KDE4_KDECORE_LIBS} kio )

install(TARGETS kdeinit_kded  DESTINATION ${LIB_INSTALL_DIR} )

target_link_libraries( kded kdeinit_kded )
install(TARGETS kded DESTINATION bin)

########### next target ###############

set(kbuildsycoca_KDEINIT_SRCS
   kbuildsycoca.cpp
   kbuildmimetypefactory.cpp
   kbuildservicetypefactory.cpp
   kbuildservicefactory.cpp
   kbuildservicegroupfactory.cpp
   #kbuildimageiofactory.cpp
   kbuildprotocolinfofactory.cpp
   kctimefactory.cpp
   vfolder_menu.cpp
   )

kde4_automoc(${kbuildsycoca_KDEINIT_SRCS})

kde4_add_kdeinit_executable( kbuildsycoca NOGUI ${kbuildsycoca_KDEINIT_SRCS})

target_link_libraries(kdeinit_kbuildsycoca  ${KDE4_KDECORE_LIBS} kio )

install(TARGETS kdeinit_kbuildsycoca  DESTINATION ${LIB_INSTALL_DIR} )

target_link_libraries( kbuildsycoca kdeinit_kbuildsycoca )
install(TARGETS kbuildsycoca DESTINATION bin)

########### next target ###############

set(kdontchangethehostname_SRCS khostname.cpp )

kde4_automoc(${kdontchangethehostname_SRCS})

kde4_add_executable(kdontchangethehostname NOGUI ${kdontchangethehostname_SRCS})

target_link_libraries(kdontchangethehostname  ${KDE4_KDECORE_LIBS} )

install(TARGETS kdontchangethehostname DESTINATION bin)


########### next target ###############

set(kde-menu_SRCS kde-menu.cpp )

kde4_automoc(${kde-menu_SRCS})

kde4_add_executable(kde-menu NOGUI ${kde-menu_SRCS})

target_link_libraries(kde-menu  ${KDE4_KDECORE_LIBS} kio)

install(TARGETS kde-menu DESTINATION bin)


########### install files ###############

install( FILES applications.menu  DESTINATION  ${SYSCONF_INSTALL_DIR}/xdg/menus )
install( FILES kdedmodule.desktop  DESTINATION  ${SERVICETYPES_INSTALL_DIR} )
install( FILES kded.upd  DESTINATION  ${DATA_INSTALL_DIR}/kconf_update )





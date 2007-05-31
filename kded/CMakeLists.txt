project(kded)

add_subdirectory( test )

include_directories( ${KDE4_KIO_INCLUDES} )

########### next target ###############

set(kded_KDEINIT_SRCS kded.cpp )

kde4_automoc(${kded_KDEINIT_SRCS})

if (WIN32)
   set (mode NOGUI)
endif (WIN32)
kde4_add_kdeinit_executable( kded4  ${mode} ${kded_KDEINIT_SRCS})

target_link_libraries(kdeinit_kded4  ${KDE4_KIO_LIBS} )

install(TARGETS kdeinit_kded4  DESTINATION ${LIB_INSTALL_DIR} )

target_link_libraries( kded4 kdeinit_kded4 )
install(TARGETS kded4 DESTINATION ${BIN_INSTALL_DIR} )

########### next target ###############

set(kbuildsycoca_KDEINIT_SRCS
   kbuildsycoca.cpp
   kbuildmimetypefactory.cpp
   kmimefileparser.cpp
   kbuildservicetypefactory.cpp
   kbuildservicefactory.cpp
   kbuildservicegroupfactory.cpp
   #kbuildimageiofactory.cpp
   kbuildprotocolinfofactory.cpp
   kctimefactory.cpp
   vfolder_menu.cpp
   )

kde4_automoc(${kbuildsycoca_KDEINIT_SRCS})

# We need to add a '4' so that kde3 apps running kbuildsycoca don't run the kde3 one.
kde4_add_kdeinit_executable( kbuildsycoca4 NOGUI ${kbuildsycoca_KDEINIT_SRCS})

target_link_libraries(kdeinit_kbuildsycoca4  ${KDE4_KIO_LIBS} )

install(TARGETS kdeinit_kbuildsycoca4  DESTINATION ${LIB_INSTALL_DIR} )

target_link_libraries( kbuildsycoca4 kdeinit_kbuildsycoca4 )
install(TARGETS kbuildsycoca4 DESTINATION ${BIN_INSTALL_DIR} )

########### next target ###############

set(kdontchangethehostname_SRCS khostname.cpp )

kde4_automoc(${kdontchangethehostname_SRCS})

kde4_add_executable(kdontchangethehostname NOGUI ${kdontchangethehostname_SRCS})

target_link_libraries(kdontchangethehostname  ${KDE4_KDECORE_LIBS} )

install(TARGETS kdontchangethehostname DESTINATION ${LIBEXEC_INSTALL_DIR} )

########### install files ###############

install( FILES applications.menu  DESTINATION  ${SYSCONF_INSTALL_DIR}/xdg/menus )
install( FILES kdedmodule.desktop  DESTINATION  ${SERVICETYPES_INSTALL_DIR} )
install( FILES kded.upd  DESTINATION  ${DATA_INSTALL_DIR}/kconf_update )



########### next target ###############

set(knotify_KDEINIT_SRCS
main.cpp
knotify.cpp
knotifyplugin.cpp
knotifyconfig.cpp
notifybysound.cpp
notifybypopup.cpp
notifybylogfile.cpp
notifybytaskbar.cpp
notifybyexecute.cpp
)

kde4_automoc(${knotify_KDEINIT_SRCS})

kde4_add_kdeinit_executable( knotify ${knotify_KDEINIT_SRCS})

target_link_libraries(kdeinit_knotify  ${KDE4_KDECORE_LIBS} kdeui phononcore )


install_targets(${LIB_INSTALL_DIR} kdeinit_knotify )

target_link_libraries( knotify kdeinit_knotify )
install(TARGETS knotify DESTINATION bin)

########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kde FILES kde.notifyrc )
install_files( ${SERVICES_INSTALL_DIR} FILES knotify.desktop )





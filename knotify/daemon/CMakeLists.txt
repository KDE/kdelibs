
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


install(TARGETS kdeinit_knotify  DESTINATION ${LIB_INSTALL_DIR} )

target_link_libraries( knotify kdeinit_knotify )
install(TARGETS knotify DESTINATION bin)

########### install files ###############

install( FILES kde.notifyrc  DESTINATION  ${DATA_INSTALL_DIR}/kde )
install( FILES knotify.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )





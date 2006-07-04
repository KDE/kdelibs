


########### next target ###############

set(cupsdconf_KDEINIT_SRCS
cupsdconf.cpp
cupsddialog.cpp
cupsdpage.cpp
cupsdcomment.cpp
cupsdsplash.cpp
cupsdserverpage.cpp
cupsdlogpage.cpp
cupsdjobspage.cpp
cupsdfilterpage.cpp
qdirlineedit.cpp
cupsddirpage.cpp
portdialog.cpp
cupsdnetworkpage.cpp
editlist.cpp
cupsdbrowsingpage.cpp
browsedialog.cpp
cupsdsecuritypage.cpp
locationdialog.cpp
addressdialog.cpp
cups-util.c
qdirmultilineedit.cpp
sizewidget.cpp
main.cpp
)

kde4_automoc(${cupsdconf_KDEINIT_SRCS})

kde4_add_kdeinit_executable( cupsdconf ${cupsdconf_KDEINIT_SRCS})

target_link_libraries(kdeinit_cupsdconf  ${KDE4_KDECORE_LIBS} kio ${CUPS_LIBRARIES})

install(TARGETS kdeinit_cupsdconf  DESTINATION ${LIB_INSTALL_DIR} )

target_link_libraries( cupsdconf kdeinit_cupsdconf )
install(TARGETS cupsdconf DESTINATION bin)

########### install files ###############

install( FILES kde_logo.png cups_logo.png cupsd.conf.template  DESTINATION  ${DATA_INSTALL_DIR}/kdeprint )





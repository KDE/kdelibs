kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


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

set( knotify_DCOP_SKEL_SRCS
knotify.h
)

kde4_add_dcop_skels(knotify_KDEINIT_SRCS ${knotify_DCOP_SKEL_SRCS})

kde4_add_kdeinit_executable( knotify ${knotify_KDEINIT_SRCS})

target_link_libraries(kdeinit_knotify  ${QT_AND_KDECORE_LIBS} kdeui )

install_targets(${KDE4_LIB_INSTALL_DIR} kdeinit_knotify )

target_link_libraries( knotify kdeinit_knotify )
install_targets(/bin knotify )

########### install files ###############

install_files( ${KDE4_DATA_DIR}/kde FILES kde.notifyrc )
install_files( ${KDE4_SERVICES_DIR} FILES knotify.desktop )

kde4_footer()



#original Makefile.am contents follow:

#
#INCLUDES= $(all_includes) $(akode_includes)
#
######## Files
#
##kde_module_LTLIBRARIES = knotify.la
#lib_LTLIBRARIES =
#kdeinit_LTLIBRARIES = knotify.la
#
#knotify_la_SOURCES	= main.cpp knotify.cpp knotify.skel knotifyplugin.cpp knotifyconfig.cpp \
# 	notifybysound.cpp notifybypopup.cpp notifybylogfile.cpp notifybytaskbar.cpp notifybyexecute.cpp
#knotify_la_LIBADD	= $(LIB_KDEUI) $(akode_libs)
#knotify_la_LDFLAGS = $(all_libraries) -module -avoid-version
#knotify_la_METASOURCES = AUTO
#
#noinst_HEADERS	=  knotify.h
#
#kdelnkdir = $(kde_servicesdir)
#kdelnk_DATA = knotify.desktop
#
#kdedatadir = $(kde_datadir)/kde
#kdedata_DATA = kde.notifyrc
#
#
#messages:
#	$(XGETTEXT) knotify.cpp -o $(podir)/knotify.pot
#

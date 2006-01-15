kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/arts/kde ${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kmcop_KDEINIT_SRCS
mcopdcopobject.cpp
kmcop.cpp
)

kde4_automoc(${kmcop_KDEINIT_SRCS})

set( kmcop_DCOP_SKEL_SRCS
kmcop.h
)

kde4_add_dcop_skels(kmcop_KDEINIT_SRCS ${kmcop_DCOP_SKEL_SRCS})

kde4_add_klm( kmcop ${kmcop_KDEINIT_SRCS})

target_link_libraries(kdeinit_kmcop  ${QT_AND_KDECORE_LIBS} artskde )

install_targets(/lib kdeinit_kmcop )

target_link_libraries( kmcop kdeinit_kmcop )
install_targets(/bin kmcop )

########### install files ###############

install_files( ${KDE4_SERVICES_DIR} FILES kmcop.desktop )

kde4_footer()



#original Makefile.am contents follow:

#
#INCLUDES= -I$(includedir)/arts -I$(top_srcdir)/arts/kde $(all_includes)
#
#bin_PROGRAMS=
#lib_LTLIBRARIES =
#kdeinit_LTLIBRARIES = kmcop.la
#
#kmcop_la_SOURCES = mcopdcopobject.cpp kmcop.cpp kmcop.skel
#kmcop_la_LIBADD	= ../libartskde.la
#kmcop_la_LDFLAGS = $(all_libraries) -module -avoid-version
#kmcop_la_METASOURCES = AUTO
#
#noinst_HEADERS = kmcop.h
#
#kdelnkdir = $(kde_servicesdir)
#kdelnk_DATA = kmcop.desktop
#
#messages:
#	$(XGETTEXT) kmcop.cpp -o $(podir)/kmcop.pot
#

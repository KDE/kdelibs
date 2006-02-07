kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/interfaces/ ${CMAKE_SOURCE_DIR}/arts/kde ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kfileaudiopreview_PART_SRCS
kfileaudiopreview.cpp
)

kde4_automoc(${kfileaudiopreview_PART_SRCS})

kde4_add_plugin(kfileaudiopreview ${kfileaudiopreview_PART_SRCS})

kde4_create_libtool_file(kfileaudiopreview / )

target_link_libraries(kfileaudiopreview  ${QT_AND_KDECORE_LIBS} kmediaplayer artskde kio )

install_targets(${KDE4_LIB_INSTALL_DIR}/kde4 kfileaudiopreview )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/interfaces/ -I$(top_srcdir)/arts/kde -I$(includedir)/arts $(all_includes)
#METASOURCES = AUTO
#
#kde_module_LTLIBRARIES = kfileaudiopreview.la
#
#kfileaudiopreview_la_SOURCES = kfileaudiopreview.cpp
#kfileaudiopreview_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -module -avoid-version -no-undefined
#kfileaudiopreview_la_LIBADD = $(top_builddir)/interfaces/kmediaplayer/libkmediaplayer.la $(top_builddir)/arts/kde/libartskde.la $(LIB_KIO)
#
#noinst_HEADERS = kfileaudiopreview.h
#
#messages:
#	$(XGETTEXT) $(kfileaudiopreview_la_SOURCES) -o $(podir)/kfileaudiopreview.pot

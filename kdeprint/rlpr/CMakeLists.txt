kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kdeprint ${CMAKE_SOURCE_DIR}/kdeprint/management ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kdeprint_rlpr_PART_SRCS
kmrlprfactory.cpp
kmrlprmanager.cpp
krlprprinterimpl.cpp
kmrlpruimanager.cpp
kmwrlpr.cpp
kmproprlpr.cpp
kmproxywidget.cpp
kmconfigproxy.cpp
)

kde4_automoc(${kdeprint_rlpr_PART_SRCS})

kde4_add_plugin(kdeprint_rlpr ${kdeprint_rlpr_PART_SRCS})

target_link_libraries(kdeprint_rlpr  ${QT_AND_KDECORE_LIBS} kdeprint_management )

kde4_install_libtool_file(${KDE4_PLUGIN_INSTALL_DIR} kdeprint_rlpr )
install_targets(${KDE4_PLUGIN_INSTALL_DIR} kdeprint_rlpr )


########### install files ###############

install_files( ${KDE4_DATA_DIR}/kdeprint/plugins FILES rlpr.print )

kde4_footer()



#original Makefile.am contents follow:

#AM_CPPFLAGS = -D_KDEPRINT_COMPILE
#
#INCLUDES= -I$(top_srcdir) -I$(top_srcdir)/kdeprint -I$(top_srcdir)/kdeprint/management $(all_includes)
#
#kde_module_LTLIBRARIES = kdeprint_rlpr.la
#
#kdeprint_rlpr_la_SOURCES = kmrlprfactory.cpp kmrlprmanager.cpp krlprprinterimpl.cpp kmrlpruimanager.cpp \
#			      kmwrlpr.cpp kmproprlpr.cpp kmproxywidget.cpp kmconfigproxy.cpp
#kdeprint_rlpr_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
#kdeprint_rlpr_la_LIBADD = $(top_builddir)/kdeprint/management/libkdeprint_management.la
#kdeprint_rlpr_la_METASOURCES = AUTO
#
#noinst_HEADERS = kmrlprmanager.h krlprprinterimpl.h kmrlpruimanager.h kmwrlpr.h kmproprlpr.h \
#		 kmproxywidget.h kmconfigproxy.h
#
#entry_DATA = rlpr.print
#entrydir = $(kde_datadir)/kdeprint/plugins

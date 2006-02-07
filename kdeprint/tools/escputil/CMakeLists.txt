kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kdeprint_tool_escputil_PART_SRCS
escpwidget.cpp
)

kde4_automoc(${kdeprint_tool_escputil_PART_SRCS})

kde4_add_plugin(kdeprint_tool_escputil ${kdeprint_tool_escputil_PART_SRCS})

kde4_create_libtool_file( kdeprint_tool_escputil /kde4/ )

target_link_libraries(kdeprint_tool_escputil  ${QT_AND_KDECORE_LIBS} kdeui )

install_targets(${KDE4_LIB_INSTALL_DIR}/kde4 kdeprint_tool_escputil )


########### install files ###############

install_files( ${KDE4_DATA_DIR}/kdeprint/tools FILES escputil.desktop )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = $(all_includes)
#
#kde_module_LTLIBRARIES = kdeprint_tool_escputil.la
#kdeprint_tool_escputil_la_SOURCES = escpwidget.cpp
#kdeprint_tool_escputil_la_METASOURCES = AUTO
#kdeprint_tool_escputil_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
#kdeprint_tool_escputil_la_LIBADD = $(LIB_KDEUI)
#
#noinst_HEADERS = escpwidget.h
#
#service_DATA = escputil.desktop
#servicedir = $(kde_datadir)/kdeprint/tools

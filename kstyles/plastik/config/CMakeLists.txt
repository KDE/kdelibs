kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kstyle_plastik_config_PART_SRCS plastikconf.cpp )

kde4_automoc(${kstyle_plastik_config_PART_SRCS})

kde4_add_plugin(kstyle_plastik_config ${kstyle_plastik_config_PART_SRCS})

target_link_libraries(kstyle_plastik_config  ${QT_AND_KDECORE_LIBS} kdeui )

kde4_install_libtool_file(${KDE4_PLUGIN_INSTALL_DIR} kstyle_plastik_config )
install_targets(${KDE4_PLUGIN_INSTALL_DIR} kstyle_plastik_config )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = $(all_includes)
#
#noinst_HEADERS = plastikconf.h
#kde_module_LTLIBRARIES = kstyle_plastik_config.la
#kstyle_plastik_config_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
#kstyle_plastik_config_la_LIBADD  = $(LIB_KDEUI)
#kstyle_plastik_config_la_SOURCES = plastikconf.cpp
#kstyle_plastik_config_la_METASOURCES = AUTO
#
#
#messages:
#	$(XGETTEXT) *.cpp *.h -o $(podir)/kstyle_plastik_config.pot

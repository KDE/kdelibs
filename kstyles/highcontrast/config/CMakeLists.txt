kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kstyle_highcontrast_config_PART_SRCS
highcontrastconfig.cpp
)

kde4_automoc(${kstyle_highcontrast_config_PART_SRCS})

kde4_add_plugin(kstyle_highcontrast_config ${kstyle_highcontrast_config_PART_SRCS})

target_link_libraries(kstyle_highcontrast_config  ${QT_AND_KDECORE_LIBS} kdeui )

install_targets(${KDE4_LIB_INSTALL_DIR}/kde4 kstyle_highcontrast_config )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = $(all_includes)
#
#noinst_HEADERS = highcontrastconfig.h
#kde_module_LTLIBRARIES = kstyle_highcontrast_config.la
#kstyle_highcontrast_config_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
#kstyle_highcontrast_config_la_LIBADD  = $(LIB_KDEUI)
#kstyle_highcontrast_config_la_SOURCES = highcontrastconfig.cpp
#kstyle_highcontrast_config_la_METASOURCES = AUTO
#
#
#messages:
#	$(XGETTEXT) *.cpp *.h -o $(podir)/kstyle_highcontrast_config.pot

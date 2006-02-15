kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kcm_spellchecking_PART_SRCS
   spellchecking.cpp
   ${kspell2base_STAT_SRCS}
)

if (WIN32)
   add_definitions(-DKSPELL2_EXPORT=)
endif (WIN32)

kde4_automoc(${kcm_spellchecking_PART_SRCS})

kde4_add_plugin(kcm_spellchecking ${kcm_spellchecking_PART_SRCS})

target_link_libraries(kcm_spellchecking  ${QT_AND_KDECORE_LIBS} kdeui kutils)

kde4_install_libtool_file(${KDE4_PLUGIN_INSTALL_DIR} kcm_spellchecking )
install_targets(${KDE4_PLUGIN_INSTALL_DIR} kcm_spellchecking )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES spellchecking.desktop )

kde4_footer()



#original Makefile.am contents follow:

#kde_module_LTLIBRARIES = kcm_spellchecking.la
#
#kcm_spellchecking_la_SOURCES = spellchecking.cpp
#
#kcm_spellchecking_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
#kcm_spellchecking_la_LIBADD = $(LIB_KDEUI) $(LIB_KDECORE) ../libkspell2base.la
#
#AM_CPPFLAGS= $(all_includes)
#
#METASOURCES = spellchecking.moc
#
#messages:
#	$(XGETTEXT) $(kcm_spellchecking_la_SOURCES) -o $(podir)/kcmspellchecking.pot
#
#xdg_apps_DATA = spellchecking.desktop
#

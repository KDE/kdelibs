
add_definitions( ${QDBUS_DEFINITIONS} )

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

target_link_libraries(kcm_spellchecking  ${KDE4_KDECORE_LIBS} kdeui kutils)

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kcm_spellchecking )
install_targets(${PLUGIN_INSTALL_DIR} kcm_spellchecking )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES spellchecking.desktop )





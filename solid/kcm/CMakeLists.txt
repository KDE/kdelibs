
include_directories( ${KDE4_KDEUI_INCLUDES} )

set(kcmsolid_SRCS kcmsolid.cpp backendchooser.cpp)
kde4_add_ui_files(kcmsolid_SRCS backendchooser.ui)
kde4_automoc(${kcmsolid_SRCS})

kde4_add_plugin(kcm_solid ${kcmsolid_SRCS})
target_link_libraries(kcm_solid  ${KDE4_KDEUI_LIBS} ${KDE4_SOLID_LIBS} )
install(TARGETS kcm_solid  DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############
install( FILES kcm_solid.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )


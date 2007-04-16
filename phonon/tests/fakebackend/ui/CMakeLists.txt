
########### next target ###############

set(phonon_fakeui_PART_SRCS videowidget.cpp backend.cpp )

kde4_automoc(${phonon_fakeui_PART_SRCS})

kde4_add_plugin(phonon_fakeui ${phonon_fakeui_PART_SRCS})


target_link_libraries(phonon_fakeui ${KDE4_PHONONCORE_LIBS} phononui)

install(TARGETS phonon_fakeui  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############


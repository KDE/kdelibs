set(phonon_fake_PART_SRCS
   abstractaudiooutput.cpp
   audiodataoutput.cpp
   delayaudioeffect.cpp
   audioeffect.cpp
   audiooutput.cpp
   audiopath.cpp
   mediaobject.cpp
   stream.cpp
   videoeffect.cpp
   brightnesscontrol.cpp
   videopath.cpp
   videodataoutput.cpp
   visualization.cpp
   backend.cpp
   volumefadereffect.cpp
   videowidget.cpp
   )


kde4_add_plugin(phonon_fake ${phonon_fake_PART_SRCS})


target_link_libraries(phonon_fake ${KDE4_KDECORE_LIBS} ${KDE4_PHONON_LIBS} phononexperimental)

install(TARGETS phonon_fake  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES  fake.desktop  DESTINATION  ${SERVICES_INSTALL_DIR}/phononbackends )



add_subdirectory( ui )

########### next target ###############

set(phonon_fake_PART_SRCS
   abstractmediaproducer.cpp
   abstractaudiooutput.cpp
   audiodataoutput.cpp
   delayaudioeffect.cpp
   audioeffect.cpp
   audiooutput.cpp
   audiopath.cpp
   bytestream.cpp
   mediaobject.cpp
   mediaqueue.cpp
   avcapture.cpp
   videoeffect.cpp
   brightnesscontrol.cpp
   videopath.cpp
   videodataoutput.cpp
   visualization.cpp
   backend.cpp
   volumefadereffect.cpp )

kde4_automoc(${phonon_fake_PART_SRCS})

kde4_add_plugin(phonon_fake ${phonon_fake_PART_SRCS})


target_link_libraries(phonon_fake ${KDE4_PHONONCORE_LIBS})

install(TARGETS phonon_fake  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES  fake.desktop  DESTINATION  ${SERVICES_INSTALL_DIR}/phononbackends )


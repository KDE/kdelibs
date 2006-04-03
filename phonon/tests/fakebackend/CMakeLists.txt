kde4_header()

add_subdirectory( ui ) 

########### next target ###############

set(phonon_fake_PART_SRCS 
   abstractmediaproducer.cpp 
   abstractaudiooutput.cpp 
   audiodataoutput.cpp 
   audioeffect.cpp 
   audiooutput.cpp 
   audiopath.cpp 
   bytestream.cpp 
   mediaobject.cpp 
   avcapture.cpp 
   videoeffect.cpp 
   videopath.cpp 
   backend.cpp 
   volumefadereffect.cpp )

kde4_automoc(${phonon_fake_PART_SRCS})

kde4_add_plugin(phonon_fake ${phonon_fake_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} phonon_fake )

target_link_libraries(phonon_fake  ${KDE4_KDECORE_LIBS} phononcore kio )

install_targets(${LIB_INSTALL_DIR}/kde4 phonon_fake )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR}/phononbackends FILES  fake.desktop )

kde4_footer()

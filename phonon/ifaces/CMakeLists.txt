kde4_header()

add_subdirectory( ui ) 

########### next target ###############

set(phononifaces_LIB_SRCS backend.cpp base.cpp )

kde4_automoc(${phononifaces_LIB_SRCS})

kde4_add_library(phononifaces SHARED ${phononifaces_LIB_SRCS})

target_link_libraries(phononifaces  ${KDE4_KDECORE_LIBS} )

set_target_properties(phononifaces PROPERTIES VERSION 0.0.0 SOVERSION 0 )
install_targets(${LIB_INSTALL_DIR} phononifaces )


########### install files ###############

install_files( /include/phonon/ifaces FILES
	abstractmediaproducer.h
	mediaobject.h
	soundcardcapture.h
	audiopath.h
	backend.h
	abstractaudiooutput.h
	audiooutput.h
	audiodataoutput.h
	videopath.h
	abstractvideooutput.h
	videoeffect.h
	audioeffect.h
	bytestream.h
	base.h
	volumefadereffect.h
	avcapture.h
	)

kde4_footer()

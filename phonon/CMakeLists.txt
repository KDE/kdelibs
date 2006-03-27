kde4_header()

# Those apply to all subdirs
include_directories( ${KDE4_KIO_INCLUDES} )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

add_subdirectory( ifaces ) 
add_subdirectory( ui ) 
add_subdirectory( tests ) 


########### next target ###############

set(phononcore_LIB_SRCS
	namedescriptiontuple.cpp
	audiooutputdevice.cpp
	audiocapturedevice.cpp
	videocapturedevice.cpp
	phononnamespace.cpp
	base.cpp
	abstractmediaproducer.cpp
	mediaobject.cpp
	avcapture.cpp
	bytestream.cpp
	audiopath.cpp
	videopath.cpp
	audioeffect.cpp
	volumefadereffect.cpp
	videoeffect.cpp
	abstractaudiooutput.cpp
	audiooutput.cpp
	audiodataoutput.cpp
	abstractvideooutput.cpp
	simpleplayer.cpp
	backendcapabilities.cpp
	factory.cpp
	)

kde4_automoc(${phononcore_LIB_SRCS})

kde4_add_dcop_skels(phononcore_LIB_SRCS factory.h mixeriface.h )

kde4_add_library(phononcore SHARED ${phononcore_LIB_SRCS})

target_link_libraries(phononcore  ${KDE4_KDECORE_LIBS} kio phononifaces )

set_target_properties(phononcore PROPERTIES VERSION 0.0.0 SOVERSION 0 )
install_targets(${LIB_INSTALL_DIR} phononcore )


########### install files ###############

install_files( /include/phonon FILES
	namedescriptiontuple.h
	audiooutputdevice.h
	audiocapturedevice.h
	videocapturedevice.h
	phononnamespace.h
	base.h
	abstractmediaproducer.h
	mediaobject.h
	avcapture.h
	bytestream.h
	audiopath.h
	videopath.h
	audioeffect.h
	volumefadereffect.h
	videoeffect.h
	mixeriface.h
	abstractaudiooutput.h
	audiooutput.h
	audiodataoutput.h
	abstractvideooutput.h
	simpleplayer.h
	backendcapabilities.h
	)
install_files( ${SERVICETYPES_INSTALL_DIR} FILES  phononbackend.desktop )

kde4_footer()

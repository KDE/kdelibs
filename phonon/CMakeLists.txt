
# Those apply to all subdirs
include_directories( ${KDE4_KIO_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )


# phonon specific configure checks
# maybe they could be moved into a separate file which is then included here, Alex

include(CheckIncludeFiles)

check_include_files(sys/soundcard.h HAVE_SYS_SOUNDCARD_H)

configure_file(config-phonon.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-phonon.h )


# and now on to the rest

add_subdirectory( ifaces )
add_subdirectory( ui )
add_subdirectory( tests )
add_subdirectory( kcm )


########### next target ###############

set(phononcore_LIB_SRCS
	ifaces/backend.cpp
	ifaces/base.cpp
	ifaces/effect.cpp
	ifaces/audioeffect.cpp
	namedescriptiontuple.cpp
	namedescriptiontuplemodel.cpp
	audiooutputdevice.cpp
	audiocapturedevice.cpp
	videooutputdevice.cpp
	videocapturedevice.cpp
	audioeffectdescription.cpp
	videoeffectdescription.cpp
	phononnamespace.cpp
	base.cpp
	abstractmediaproducer.cpp
	mediaobject.cpp
	avcapture.cpp
	bytestream.cpp
	audiopath.cpp
	videopath.cpp
	effect.cpp
	effectparameter.cpp
	audioeffect.cpp
	volumefadereffect.cpp
	videoeffect.cpp
	abstractaudiooutput.cpp
	audiooutput.cpp
	audiodataoutput.cpp
	abstractvideooutput.cpp
	videodataoutput.cpp
	simpleplayer.cpp
	backendcapabilities.cpp
	factory.cpp
	)

kde4_automoc(${phononcore_LIB_SRCS})

kde4_add_dcop_skels(phononcore_LIB_SRCS factory.h mixeriface.h )

kde4_add_library(phononcore SHARED ${phononcore_LIB_SRCS})

target_link_libraries(phononcore  ${KDE4_KDECORE_LIBS} kio )

set_target_properties(phononcore PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install_targets(${LIB_INSTALL_DIR} phononcore )


########### install files ###############

install_files( /include/phonon FILES
	namedescriptiontuple.h
	namedescriptiontuplemodel.h
	audiooutputdevice.h
	audiocapturedevice.h
	videooutputdevice.h
	videocapturedevice.h
	audioeffectdescription.h
	videoeffectdescription.h
	phononnamespace.h
	base.h
	basedestructionhandler.h
	abstractmediaproducer.h
	mediaobject.h
	avcapture.h
	bytestream.h
	audiopath.h
	videopath.h
	effect.h
	effectparameter.h
	audioeffect.h
	volumefadereffect.h
	videoeffect.h
	mixeriface.h
	abstractaudiooutput.h
	audiooutput.h
	audiodataoutput.h
	abstractvideooutput.h
	videoframe.h
	videodataoutput.h
	simpleplayer.h
	backendcapabilities.h
	phonondefs.h
	)
install_files( ${SERVICETYPES_INSTALL_DIR} FILES  phononbackend.desktop )


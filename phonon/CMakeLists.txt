
add_definitions(${QDBUS_DEFINITIONS})

# Those apply to all subdirs
include_directories( ${KDE4_KIO_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )


# phonon specific configure checks
# maybe they could be moved into a separate file which is then included here, Alex

include(CheckIncludeFiles)

check_include_files(sys/soundcard.h HAVE_SYS_SOUNDCARD_H)

configure_file(config-phonon.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-phonon.h )


# and now on to the rest

add_subdirectory( ui )
add_subdirectory( tests )
add_subdirectory( kcm )


########### next target ###############

set(phononcore_LIB_SRCS
	namedescriptiontuple.cpp
	namedescriptiontuplemodel.cpp
	audiooutputdevice.cpp
	audiocapturedevice.cpp
	videooutputdevice.cpp
	videocapturedevice.cpp
	audioeffectdescription.cpp
	videoeffectdescription.cpp
	visualizationeffect.cpp
	audiocodec.cpp
	videocodec.cpp
	containerformat.cpp
	phononnamespace.cpp
	base.cpp
	abstractmediaproducer.cpp
	mediaobject.cpp
	mediaqueue.cpp
	avcapture.cpp
	bytestream.cpp
	audiopath.cpp
	videopath.cpp
	effect.cpp
	effectparameter.cpp
	audioeffect.cpp
	volumefadereffect.cpp
	videoeffect.cpp
	mixeradaptor.cpp
	brightnesscontrol.cpp
	abstractaudiooutput.cpp
	audiooutput.cpp
	audiodataoutput.cpp
	abstractvideooutput.cpp
	videodataoutput.cpp
	visualization.cpp
	simpleplayer.cpp
	backendcapabilities.cpp
	factory.cpp
	)

kde4_automoc(${phononcore_LIB_SRCS})

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
	visualizationeffect.h
	audiocodec.h
	videocodec.h
	containerformat.h
	phononnamespace.h
	base.h
	basedestructionhandler.h
	abstractmediaproducer.h
	mediaobject.h
	mediaqueue.h
	avcapture.h
	bytestream.h
	audiopath.h
	videopath.h
	effect.h
	effectparameter.h
	audioeffect.h
	volumefadereffect.h
	videoeffect.h
	mixeradaptor.h
	brightnesscontrol.h
	abstractaudiooutput.h
	audiooutput.h
	audiodataoutput.h
	abstractvideooutput.h
	videoframe.h
	videodataoutput.h
	visualization.h
	simpleplayer.h
	backendcapabilities.h
	phonondefs.h
	)
install_files( ${SERVICETYPES_INSTALL_DIR} FILES  phononbackend.desktop )


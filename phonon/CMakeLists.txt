
KDE4_NO_ENABLE_FINAL(phonon)

# Those apply to all subdirs
include_directories(${KDE4_KIO_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR}/solid)


# phonon specific configure checks
# maybe they could be moved into a separate file which is then included here, Alex

include(CheckIncludeFiles)

check_include_files(sys/soundcard.h HAVE_SYS_SOUNDCARD_H)

configure_file(config-phonon.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-phonon.h )


# and now on to the rest

add_subdirectory( ui )
add_subdirectory( tests )
add_subdirectory( kcm )
add_subdirectory( examples )

########### next target ###############

set(phononcore_LIB_SRCS
	objectdescription.cpp
	objectdescriptionmodel.cpp
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
	audiooutputadaptor.cpp
	brightnesscontrol.cpp
	deinterlacefilter.cpp
	abstractaudiooutput.cpp
	audiooutput.cpp
	audiodataoutput.cpp
	abstractvideooutput.cpp
	videodataoutput.cpp
	visualization.cpp
	audioplayer.cpp
	backendcapabilities.cpp
	globalconfig.cpp
	factory.cpp
	kiofallback.cpp
	)

kde4_automoc(${phononcore_LIB_SRCS})

kde4_add_library(phononcore SHARED ${phononcore_LIB_SRCS})

target_link_libraries(phononcore ${KDE4_KIO_LIBS})

set_target_properties(phononcore PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS phononcore  DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############

install( FILES
	objectdescription.h
	objectdescriptionmodel.h
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
	brightnesscontrol.h
	deinterlacefilter.h
	abstractaudiooutput.h
	audiooutput.h
	audiodataoutput.h
	abstractvideooutput.h
	videoframe.h
	videodataoutput.h
	visualization.h
	audioplayer.h
	backendcapabilities.h
	phonondefs.h
	bytestreaminterface.h
	backendinterface.h
	mediaproducerinterface.h
	mediaobjectinterface.h
	audiooutputinterface.h
	DESTINATION ${INCLUDE_INSTALL_DIR}/phonon)
install( FILES  phononbackend.desktop  DESTINATION  ${SERVICETYPES_INSTALL_DIR} )
install( FILES org.kde.Phonon.AudioOutput.xml DESTINATION ${DBUS_INTERFACES_DIR} )
install(FILES phonon.notifyrc DESTINATION ${DATA_INSTALL_DIR}/phonon)


########################## libkaudiodevicelist ####################################

set(kaudiodevicelist_LIB_SRCS
	audiodevice.cpp
	audiodeviceenumerator.cpp
	)
kde4_automoc(${kaudiodevicelist_LIB_SRCS})
kde4_add_library(kaudiodevicelist SHARED ${kaudiodevicelist_LIB_SRCS})
target_link_libraries(kaudiodevicelist ${KDE4_SOLID_LIBS})

set_target_properties(kaudiodevicelist PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})

install(TARGETS kaudiodevicelist DESTINATION ${LIB_INSTALL_DIR})
install(FILES
	audiodevice.h
	audiodeviceenumerator.h
	DESTINATION ${INCLUDE_INSTALL_DIR}/phonon)

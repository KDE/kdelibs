# Those apply to all subdirs
include_directories(${KDE4_KDECORE_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR}/solid)

FIND_PACKAGE(Alsa)
ALSA_CONFIGURE_FILE(${CMAKE_CURRENT_BINARY_DIR}/config-alsa.h)
ALSA_VERSION_STRING(ALSA_VERSION_STR)
IF(ALSA_VERSION_STR AND NOT WIN32)
    SET(ALSA_VERSION_STR_Z "${ALSA_VERSION_STR}z") # append z so that 1.0.14 is greater than 1.0.14rc1
    SET(RECOMMENDED_MIN_VERSION "1.0.14rc1")
    STRING(COMPARE LESS ${RECOMMENDED_MIN_VERSION} ${ALSA_VERSION_STR_Z} VERSION_LESS)
    IF(VERSION_LESS)
        MESSAGE(STATUS "libasound version '${ALSA_VERSION_STR}' is recent enough")
    ELSE(VERSION_LESS)
        MESSAGE(STATUS "WARNING: libasound version '${ALSA_VERSION_STR}' is not recent enough. The recommended minimum version is '${RECOMMENDED_MIN_VERSION}'. Older versions will compile correctly, but break on dmix usage.")
    ENDIF(VERSION_LESS)
ENDIF(ALSA_VERSION_STR AND NOT WIN32)

add_subdirectory( ui )
add_subdirectory( tests )
add_subdirectory( kcm )
add_subdirectory( examples )
add_subdirectory(experimental)

########### next target ###############

set(phononcore_LIB_SRCS
	objectdescription.cpp
	objectdescriptionmodel.cpp
	phononnamespace.cpp
	base.cpp
	abstractmediaproducer.cpp
	abstractmediaproducer_p.cpp
	mediaobject.cpp
	mediaqueue.cpp
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
	abstractaudiooutput_p.cpp
	audiooutput.cpp
	abstractvideooutput.cpp
	abstractvideooutput_p.cpp
	audioplayer.cpp
	backendcapabilities.cpp
	globalconfig.cpp
	factory.cpp
	kiofallback.cpp
	guiinterface.cpp
	trackinterface.cpp
	chapterinterface.cpp
	navigationinterface.cpp
	angleinterface.cpp
	)

kde4_automoc(${phononcore_LIB_SRCS})

kde4_add_library(phononcore SHARED ${phononcore_LIB_SRCS})

target_link_libraries(phononcore ${KDE4_KDECORE_LIBS})

set_target_properties(phononcore PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS phononcore  DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############

install( FILES
        phonon_export.h
	objectdescription.h
	objectdescriptionmodel.h
	phononnamespace.h
	base.h
	basedestructionhandler.h
	abstractmediaproducer.h
	mediaobject.h
	mediaqueue.h
	bytestream.h
	trackinterface.h
	chapterinterface.h
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
	abstractvideooutput.h
	audioplayer.h
	backendcapabilities.h
	phonondefs.h
	bytestreaminterface.h
	backendinterface.h
	mediaproducerinterface.h
	mediaobjectinterface.h
	audiooutputinterface.h
	addoninterface.h
	trackinterface.h
	chapterinterface.h
	navigationinterface.h
	angleinterface.h
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

# vim: noet

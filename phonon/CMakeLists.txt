# Those apply to all subdirs
include_directories(${KDE4_KDECORE_INCLUDES} ${CMAKE_BINARY_DIR}/solid)

add_subdirectory( tests )
add_subdirectory(examples)
add_subdirectory(experimental)
add_subdirectory(platform_kde)
if(CMAKE_SYSTEM_NAME MATCHES Linux)
add_subdirectory(libkaudiodevicelist)
add_subdirectory( kcm )
endif(CMAKE_SYSTEM_NAME MATCHES Linux)
########### next target ###############

set(phonon_LIB_SRCS
	objectdescription.cpp
	objectdescriptionmodel.cpp
	phononnamespace.cpp
	mediasource.cpp
	abstractmediastream.cpp
	streaminterface.cpp
	mediaobject.cpp
	audiopath.cpp
	videopath.cpp
	effectparameter.cpp
	effect.cpp
	volumefadereffect.cpp
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
	mediacontroller.cpp
	videowidget.cpp
	videoplayer.cpp
	seekslider.cpp
	volumeslider.cpp
	effectwidget.cpp
	iodevicestream.cpp
	)
option(PHONON_BUILD_MODELTEST  "Build Modeltest into libphonon's ObjectDescriptionModel. You need to have modeltest.h and modeltest.cpp from labs.trolltech.com for this to work. (Enabling this option effectively makes libphonon GPL).")
IF(PHONON_BUILD_MODELTEST)
set(phonon_LIB_SRCS ${phonon_LIB_SRCS} modeltest.cpp)
add_definitions(-DUSE_TROLLTECH_LABS_MODELTEST)
ENDIF(PHONON_BUILD_MODELTEST)


kde4_add_library(phonon SHARED ${phonon_LIB_SRCS})

target_link_libraries(phonon ${QT_QTDBUS_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})

set_target_properties(phonon PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS phonon  DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############

install( FILES
	phonon_export.h
	objectdescription.h
	objectdescriptionmodel.h
	phononnamespace.h
	mediasource.h
	abstractmediastream.h
	streaminterface.h
	mediaobject.h
	audiopath.h
	videopath.h
	effectparameter.h
	effect.h
	volumefadereffect.h
	brightnesscontrol.h
	deinterlacefilter.h
	abstractaudiooutput.h
	audiooutput.h
	abstractvideooutput.h
	audioplayer.h
	backendcapabilities.h
	phonondefs.h
	backendinterface.h
	mediaobjectinterface.h
	audiooutputinterface.h
	addoninterface.h
	mediacontroller.h
	videowidget.h
	videoplayer.h
	seekslider.h
	volumeslider.h
	effectwidget.h
	DESTINATION ${INCLUDE_INSTALL_DIR}/phonon)
install( FILES  phononbackend.desktop  DESTINATION  ${SERVICETYPES_INSTALL_DIR} )
install( FILES org.kde.Phonon.AudioOutput.xml DESTINATION ${DBUS_INTERFACES_DIR} )
install(FILES phonon.notifyrc DESTINATION ${DATA_INSTALL_DIR}/phonon)

# vim: noet

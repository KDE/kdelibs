include_directories(${KDE4_KIO_INCLUDES})
add_subdirectory( tests )

########### libphononexperimental ###############
set(phononexperimental_LIB_SRCS
    audiodataoutput.cpp
    videodataoutput.cpp
    visualization.cpp
    mediasource.cpp
    backendcapabilities.cpp
    factory.cpp
    globalconfig.cpp
    ../abstractaudiooutput_p.cpp
    ../abstractvideooutput_p.cpp
   )
kde4_add_library(phononexperimental SHARED ${phononexperimental_LIB_SRCS})
target_link_libraries(phononexperimental ${KDE4_PHONON_LIBS})
#do not use GENERIC versioning in phonon
set_target_properties(phononexperimental PROPERTIES VERSION 4.2.0 SOVERSION 4)
install(TARGETS phononexperimental  DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install headers ###############
install(FILES
    export.h
    audiodataoutput.h
    videoframe.h
    videodataoutput.h
    visualization.h
    mediasource.h
    backendcapabilities.h
    backendinterface.h
    factory.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/phonon/experimental)

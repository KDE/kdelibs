include_directories(${KDE4_KIO_INCLUDES})
add_subdirectory( tests )

########### libphononexperimental ###############
set(phononexperimental_LIB_SRCS
    overlayapi.cpp
    audiodataoutput.cpp
    videodataoutput.cpp
    visualization.cpp
    ../abstractaudiooutput_p.cpp
    ../abstractvideooutput_p.cpp
   )
kde4_automoc(${phononexperimental_LIB_SRCS})
kde4_add_library(phononexperimental SHARED ${phononexperimental_LIB_SRCS})
target_link_libraries(phononexperimental ${KDE4_PHONON_LIBS})
set_target_properties(phononexperimental PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS phononexperimental DESTINATION ${LIB_INSTALL_DIR})

########### install headers ###############
install(FILES
    overlayapi.h
    export.h
    audiodataoutput.h
    videoframe.h
    videodataoutput.h
    visualization.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/phonon/experimental)

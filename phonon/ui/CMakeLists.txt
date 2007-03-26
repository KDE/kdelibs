include_directories(${KDE4_KIO_INCLUDES})
add_subdirectory( tests )

########### libphononui ###############
set(phononui_LIB_SRCS
   factory.cpp
   videowidget.cpp
   videoplayer.cpp
   seekslider.cpp
   mediacontrols.cpp
   volumeslider.cpp
   effectwidget.cpp
   guiimpl.cpp
   kiofallbackimpl.cpp
   )
kde4_automoc(${phononui_LIB_SRCS})
kde4_add_library(phononui SHARED ${phononui_LIB_SRCS})
target_link_libraries(phononui ${KDE4_PHONONCORE_LIBS} ${KDE4_KIO_LIBS})
set_target_properties(phononui PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS phononui  DESTINATION ${LIB_INSTALL_DIR} )

########### libphononexperimental ###############
set(phononexperimental_LIB_SRCS
    overlayapi.cpp
   )
kde4_automoc(${phononexperimental_LIB_SRCS})
kde4_add_library(phononexperimental SHARED ${phononexperimental_LIB_SRCS})
target_link_libraries(phononexperimental ${KDE4_PHONONUI_LIBS})
set_target_properties(phononexperimental PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS phononexperimental DESTINATION ${LIB_INSTALL_DIR})

########### install headers ###############
install( FILES
	videowidget.h
	videoplayer.h
	seekslider.h
	mediacontrols.h
	volumeslider.h
	effectwidget.h
	DESTINATION ${INCLUDE_INSTALL_DIR}/phonon/ui)

install(FILES
    overlayapi.h
    export.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/phonon/experimental)

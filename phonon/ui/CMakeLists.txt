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

########### install headers ###############
install( FILES
	videowidget.h
	videoplayer.h
	seekslider.h
	mediacontrols.h
	volumeslider.h
	effectwidget.h
	DESTINATION ${INCLUDE_INSTALL_DIR}/phonon/ui)

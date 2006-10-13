
add_subdirectory( tests )

########### next target ###############

set(phononui_LIB_SRCS
   factory.cpp
   videowidget.cpp
   videoplayer.cpp
   seekslider.cpp
   mediacontrols.cpp
   volumeslider.cpp
   effectwidget.cpp
   )

kde4_automoc(phononui ${phononui_LIB_SRCS})

kde4_add_library(phononui SHARED ${phononui_LIB_SRCS})

target_link_libraries(phononui  ${KDE4_KDECORE_LIBS} phononcore )

set_target_properties(phononui PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS phononui  DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############

install( FILES
	videowidget.h
	videoplayer.h
	seekslider.h
	mediacontrols.h
	volumeslider.h
	effectwidget.h
	DESTINATION ${INCLUDE_INSTALL_DIR}/phonon/ui)



add_subdirectory( tests )

########### next target ###############

set(phononui_LIB_SRCS
   factory.cpp
   videowidget.cpp
   seekslider.cpp
   mediacontrols.cpp
   volumeslider.cpp
   effectwidget.cpp
   )

kde4_automoc(${phononui_LIB_SRCS})

kde4_add_library(phononui SHARED ${phononui_LIB_SRCS})

target_link_libraries(phononui  ${KDE4_KDECORE_LIBS} phononcore )

set_target_properties(phononui PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install_targets(${LIB_INSTALL_DIR} phononui )


########### install files ###############

install_files( /include/phonon/ui FILES
	videowidget.h
	seekslider.h
	mediacontrols.h
	volumeslider.h
	effectwidget.h
	)


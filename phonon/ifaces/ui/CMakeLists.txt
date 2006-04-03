kde4_header()


########### next target ###############

set(phononifacesui_LIB_SRCS backend.cpp )

kde4_automoc(${phononifacesui_LIB_SRCS})

kde4_add_library(phononifacesui SHARED ${phononifacesui_LIB_SRCS})

target_link_libraries(phononifacesui  ${KDE4_KDECORE_LIBS} phononcore )

set_target_properties(phononifacesui PROPERTIES VERSION 0.0.0 SOVERSION 0 )
install_targets(${LIB_INSTALL_DIR} phononifacesui )


########### install files ###############

install_files( /include/phonon/ifaces/ui FILES
	videowidget.h
	backend.h
	)

kde4_footer()

kde4_header()

add_subdirectory( tests ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(phononui_LIB_SRCS 
   factory.cpp 
   videowidget.cpp 
   seekslider.cpp 
   mediacontrols.cpp 
   volumeslider.cpp )

kde4_automoc(${phononui_LIB_SRCS})

kde4_add_library(phononui SHARED ${phononui_LIB_SRCS})

target_link_libraries(phononui  ${QT_AND_KDECORE_LIBS} phononcore phononifacesui )

set_target_properties(phononui PROPERTIES VERSION 0.0.0 SOVERSION 0 )
install_targets(${LIB_INSTALL_DIR} phononui )


########### install files ###############

install_files( /include/phonon/ui FILES  videowidget.h seekslider.h mediacontrols.h  						volumeslider.h )

kde4_footer()

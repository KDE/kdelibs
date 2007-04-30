
include_directories( ${CMAKE_SOURCE_DIR} )
include_directories( ${KDE4_KIO_INCLUDES} )


########### next target ###############

set(kfileaudiopreview_PART_SRCS
kfileaudiopreview.cpp
mediacontrols.cpp
)

kde4_automoc(${kfileaudiopreview_PART_SRCS})

kde4_add_plugin(kfileaudiopreview ${kfileaudiopreview_PART_SRCS})

target_link_libraries(kfileaudiopreview ${KDE4_KIO_LIBS} ${KDE4_PHONON_LIBS})

install(TARGETS kfileaudiopreview  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############



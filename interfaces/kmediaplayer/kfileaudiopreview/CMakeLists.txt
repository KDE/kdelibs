
include_directories(${PHONON_INCLUDE_DIR} ${KDE4_KIO_INCLUDES})
include_directories( ${ki18n_SOURCE_DIR}/src )
include_directories( ${ki18n_BINARY_DIR}/src )


########### next target ###############

set(kfileaudiopreview_PART_SRCS
kfileaudiopreview.cpp
mediacontrols.cpp
)


kde4_add_plugin(kfileaudiopreview ${kfileaudiopreview_PART_SRCS})

target_link_libraries(kfileaudiopreview ${KDE4_KIO_LIBS} ${PHONON_LIBRARIES} kwidgets ki18n)

install(TARGETS kfileaudiopreview  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############



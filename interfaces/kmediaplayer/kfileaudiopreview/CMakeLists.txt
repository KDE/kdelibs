kde4_header()

include_directories( ${CMAKE_SOURCE_DIR} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kfileaudiopreview_PART_SRCS
kfileaudiopreview.cpp
)

kde4_automoc(${kfileaudiopreview_PART_SRCS})

kde4_add_plugin(kfileaudiopreview ${kfileaudiopreview_PART_SRCS})

target_link_libraries(kfileaudiopreview  ${KDE4_KDECORE_LIBS} phononui kio )

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kfileaudiopreview )
install_targets(${PLUGIN_INSTALL_DIR} kfileaudiopreview )


########### install files ###############


kde4_footer()

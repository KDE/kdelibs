
project(kstyle-highcolor)
add_definitions(-DQT_PLUGIN)

########### next target ###############

set(highcolor_PART_SRCS highcolor.cpp )

kde4_automoc(${highcolor_PART_SRCS})

kde4_add_plugin(highcolor ${highcolor_PART_SRCS})

target_link_libraries(highcolor  ${KDE4_KDECORE_LIBS} kdefx )

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} highcolor )
install_targets(${PLUGIN_INSTALL_DIR} highcolor )


########### install files ###############






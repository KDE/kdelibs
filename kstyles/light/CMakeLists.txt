
project(kstyle-light)
add_definitions(-DQT_PLUGIN)


########### next target ###############

set(light_PART_SRCS
   light.cpp
   lightstyle-v2.cpp
   lightstyle-v3.cpp
)

kde4_automoc(${light_PART_SRCS})

kde4_add_plugin(light ${light_PART_SRCS})

target_link_libraries(light  ${KDE4_KDECORE_LIBS} kdefx )

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} light )
install_targets(${PLUGIN_INSTALL_DIR} light )


########### install files ###############






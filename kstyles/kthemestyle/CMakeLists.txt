
project(kstyle-kthemestyle)

add_definitions(-DQT_PLUGIN)


########### next target ###############

set(kthemestyle_PART_SRCS
   kthemebase.cpp
   kthemestyle.cpp
   kstyledirs.cpp
)

kde4_automoc(${kthemestyle_PART_SRCS})

kde4_add_plugin(kthemestyle ${kthemestyle_PART_SRCS})

target_link_libraries(kthemestyle  ${KDE4_KDECORE_LIBS} kdefx kdecore )

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kthemestyle )
install(TARGETS kthemestyle  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############






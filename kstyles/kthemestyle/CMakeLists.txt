
project(kstyle-kthemestyle)

add_definitions(-DQT_PLUGIN)


########### next target ###############

set(kthemestyle_PART_SRCS
   kthemebase.cpp
   kthemestyle.cpp
   kstyledirs.cpp
)

kde4_automoc(kthemestyle ${kthemestyle_PART_SRCS})

kde4_add_plugin(kthemestyle ${kthemestyle_PART_SRCS})

target_link_libraries(kthemestyle  ${KDE4_KDECORE_LIBS} kdefx kdecore )

install(TARGETS kthemestyle  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############






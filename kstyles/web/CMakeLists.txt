
project(kstyle-web)



########### next target ###############

set(webstyle_LIB_SRCS webstyle.cpp plugin.cpp )

kde4_automoc(${webstyle_LIB_SRCS})

kde4_add_library(webstyle SHARED ${webstyle_LIB_SRCS})

target_link_libraries(webstyle  ${KDE4_KDECORE_LIBS} kdeui )

set_target_properties(webstyle PROPERTIES VERSION 0.0.0 SOVERSION 0 )
install(TARGETS webstyle  DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############

install( FILES web.themerc  DESTINATION  ${DATA_INSTALL_DIR}/kstyle/themes )






add_subdirectory( tests ) 

include_directories( ${CMAKE_SOURCE_DIR}/khtml ${CMAKE_SOURCE_DIR}/kio/kssl )
include_directories( ${CMAKE_BINARY_DIR}/kio/kssl )


########### next target ###############


########### next target ###############

set(kjavaappletviewer_PART_SRCS
   kjavaapplet.cpp
   kjavaappletcontext.cpp
   kjavaappletserver.cpp
   kjavaappletwidget.cpp
   kjavaprocess.cpp
   kjavadownloader.cpp
   kjavaappletviewer.cpp
)

kde4_automoc(${kjavaappletviewer_PART_SRCS})

kde4_add_plugin(kjavaappletviewer ${kjavaappletviewer_PART_SRCS})

target_link_libraries(kjavaappletviewer  ${KDE4_KDECORE_LIBS} kparts )

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kjavaappletviewer )
install_targets(${PLUGIN_INSTALL_DIR} kjavaappletviewer )

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kjava.policy.in.cmake ${CMAKE_CURRENT_BINARY_DIR}/kjava.policy )
#macro_additional_clean_files( ${CMAKE_CURRENT_BINARY_DIR}/kjava.policy )
add_definitions(-DMAKE_KHTML_LIB)

########### install files ###############

# kjava.policy 
install_files( ${DATA_INSTALL_DIR}/kjava/ FILES kjava.jar kjava.policy pluginsinfo )
install_files( ${SERVICES_INSTALL_DIR} FILES kjavaappletviewer.desktop )
kde4_install_icons( ${DATA_INSTALL_DIR}/kjava/icons crystalsvg )





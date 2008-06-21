
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


kde4_add_plugin(kjavaappletviewer ${kjavaappletviewer_PART_SRCS})

target_link_libraries(kjavaappletviewer ${KDE4_KPARTS_LIBS})

install(TARGETS kjavaappletviewer  DESTINATION ${PLUGIN_INSTALL_DIR} )

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kjava.policy.in.cmake ${CMAKE_CURRENT_BINARY_DIR}/kjava.policy )
#macro_additional_clean_files( ${CMAKE_CURRENT_BINARY_DIR}/kjava.policy )
add_definitions(-DMAKE_KHTML_LIB)

########### install files ###############

# kjava.policy 
install( FILES kjava.jar ${CMAKE_CURRENT_BINARY_DIR}/kjava.policy pluginsinfo  DESTINATION  ${DATA_INSTALL_DIR}/kjava/ )
install( FILES kjavaappletviewer.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
kde4_install_icons( ${DATA_INSTALL_DIR}/kjava/icons  )





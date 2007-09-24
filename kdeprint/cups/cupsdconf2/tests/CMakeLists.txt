set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. )

########### cupsdconftest ###############
kde4_add_executable( cupsdconftest TEST  main.cpp )

target_link_libraries( cupsdconftest cupsdconf )




project(kwallet)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} ${KDE4_KDECORE_INCLUDES} )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/client ${CMAKE_CURRENT_BUILD_DIR}/client)

add_subdirectory( client ) 
add_subdirectory( backend ) 
add_subdirectory( tests ) 





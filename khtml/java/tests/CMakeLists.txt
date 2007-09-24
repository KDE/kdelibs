set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${CMAKE_SOURCE_DIR}/khtml/java ${CMAKE_SOURCE_DIR}/khtml )


########### next target ###############

set(testserver_SRCS testkjavaappletserver.cpp )


kde4_add_executable(testserver TEST ${testserver_SRCS})

target_link_libraries(testserver  ${KDE4_KDECORE_LIBS} khtml)




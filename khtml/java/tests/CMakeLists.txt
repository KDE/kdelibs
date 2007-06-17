
include_directories( ${CMAKE_SOURCE_DIR}/khtml/java ${CMAKE_SOURCE_DIR}/khtml )


########### next target ###############

set(testserver_SRCS testkjavaappletserver.cpp )

kde4_automoc(${testserver_SRCS})

kde4_add_test_executable(testserver ${testserver_SRCS})

target_link_libraries(testserver  ${KDE4_KDECORE_LIBS} khtml)




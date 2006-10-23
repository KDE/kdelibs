

if(KDE4_BUILD_TESTS)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

########### solidhwtest ###############

set(solidhwtest_SRCS 
   solidhwtest.cpp )

kde4_automoc(${solidhwtest_SRCS})

kde4_add_executable(solidhwtest RUN_UNINSTALLED ${solidhwtest_SRCS})
add_definitions(-DFAKE_COMPUTER_XML="\\"${CMAKE_CURRENT_SOURCE_DIR}/../backends/fakehw/fakecomputer.xml\\"")

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../backends/fakehw )

target_link_libraries(solidhwtest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} solid solidifaces solid_fakehw_static)

add_test(solidhwtest ${EXECUTABLE_OUTPUT_PATH}/solidhwtest)

########### solidnettest ###############

set(solidnettest_SRCS 
   solidnettest.cpp )

kde4_automoc(${solidnettest_SRCS})

kde4_add_executable(solidnettest RUN_UNINSTALLED ${solidnettest_SRCS})
add_definitions(-DFAKE_NETWORKING_XML="\\"${CMAKE_CURRENT_SOURCE_DIR}/../backends/fakenet/fakenetworking.xml\\"")

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../backends/fakenet )

target_link_libraries(solidnettest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} solid solidifaces solid_fakenet_static)

add_test(solidnettest ${EXECUTABLE_OUTPUT_PATH}/solidnettest)

########### install files ###############

endif(KDE4_BUILD_TESTS)


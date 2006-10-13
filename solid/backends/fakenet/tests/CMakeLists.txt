

if(KDE4_BUILD_TESTS)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. )

########### next target ###############

set(fakenetworktest_SRCS fakenetworktest.cpp )

kde4_automoc(fakenetworktest ${fakenetworktest_SRCS})

kde4_add_executable(fakenetworktest RUN_UNINSTALLED ${fakenetworktest_SRCS})

target_link_libraries(fakenetworktest solid_fakenet_static ${QT_QTTEST_LIBRARY} )
add_definitions(-DTEST_DATA="\\"${CMAKE_CURRENT_SOURCE_DIR}/../fakenetworking.xml\\"")

add_test(fakenetworktest ${EXECUTABLE_OUTPUT_PATH}/fakenetworktest)

endif(KDE4_BUILD_TESTS)

########### install files ###############




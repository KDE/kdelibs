

if(KDE4_BUILD_TESTS)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. )

########### next target ###############

set(fakehardwaretest_SRCS fakehardwaretest.cpp )

kde4_automoc(fakehardwaretest ${fakehardwaretest_SRCS})

kde4_add_executable(fakehardwaretest RUN_UNINSTALLED ${fakehardwaretest_SRCS})

target_link_libraries(fakehardwaretest  solid_fakehw_static ${QT_QTTEST_LIBRARY} )
add_definitions(-DTEST_DATA="\\"${CMAKE_CURRENT_SOURCE_DIR}/../fakecomputer.xml\\"")

add_test(fakehardwaretest ${EXECUTABLE_OUTPUT_PATH}/fakehardwaretest)

endif(KDE4_BUILD_TESTS)

########### install files ###############




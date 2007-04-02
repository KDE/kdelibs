

if(KDE4_BUILD_TESTS)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. )

########### next target ###############

set(fakebluetoothtest_SRCS fakebluetoothtest.cpp )

kde4_automoc(${fakebluetoothtest_SRCS})

kde4_add_executable(fakebluetoothtest RUN_UNINSTALLED ${fakebluetoothtest_SRCS})

target_link_libraries(fakebluetoothtest solid_fakebluetooth_static ${QT_QTTEST_LIBRARY} )
add_definitions(-DTEST_DATA="\\"${CMAKE_CURRENT_SOURCE_DIR}/../fakebluetooth.xml\\"")

add_test(fakebluetoothtest ${EXECUTABLE_OUTPUT_PATH}/fakebluetoothtest)

endif(KDE4_BUILD_TESTS)

########### install files ###############




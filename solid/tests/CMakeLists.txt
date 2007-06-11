

if(KDE4_BUILD_TESTS)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

########### fakehardwaretest ###############

set(fakehardwaretest_SRCS fakehardwaretest.cpp )

kde4_automoc(${fakehardwaretest_SRCS})

kde4_add_executable(fakehardwaretest RUN_UNINSTALLED ${fakehardwaretest_SRCS})

if(WIN32)
  set_target_properties(fakehardwaretest PROPERTIES COMPILE_FLAGS -DSOLID_EXPORT=)
endif(WIN32)

target_link_libraries(fakehardwaretest solid_static ${QT_QTCORE_LIBRARY} ${QT_QTDBUS_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTTEST_LIBRARY} )
add_definitions(-DTEST_DATA="\\"${CMAKE_CURRENT_SOURCE_DIR}/../solid/backends/fakehw/fakecomputer.xml\\"")

add_test(fakehardwaretest ${EXECUTABLE_OUTPUT_PATH}/fakehardwaretest)


########### halbasictest ###############

set(halbasictest_SRCS halbasictest.cpp )

kde4_automoc(${halbasictest_SRCS})


kde4_add_executable(halbasictest RUN_UNINSTALLED ${halbasictest_SRCS})

if(WIN32)
  set_target_properties(halbasictest PROPERTIES COMPILE_FLAGS -DSOLID_EXPORT=)
endif(WIN32)

target_link_libraries(halbasictest solid_static ${QT_QTCORE_LIBRARY} ${QT_QTDBUS_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTTEST_LIBRARY} )

add_test(halbasictest ${EXECUTABLE_OUTPUT_PATH}/halbasictest)


########### solidhwtest ###############

set(solidhwtest_SRCS 
   solidhwtest.cpp )

kde4_automoc(${solidhwtest_SRCS})

kde4_add_executable(solidhwtest NOGUI RUN_UNINSTALLED ${solidhwtest_SRCS})
add_definitions(-DFAKE_COMPUTER_XML="\\"${CMAKE_CURRENT_SOURCE_DIR}/../solid/backends/fakehw/fakecomputer.xml\\"")

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../solid/backends/fakehw )

if(WIN32)
  set_target_properties(solidhwtest PROPERTIES COMPILE_FLAGS -DSOLID_EXPORT=)
endif(WIN32)

target_link_libraries(solidhwtest ${QT_QTCORE_LIBRARY} ${QT_QTDBUS_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTTEST_LIBRARY} solid_static)

add_test(solidhwtest ${EXECUTABLE_OUTPUT_PATH}/solidhwtest)

########### solidnettestdbusservice ###############

set(solidnettestdbusservice_SRCS
   solidnettestdbusservice.cpp )

kde4_automoc(${solidnettestdbusservice_SRCS})

QT4_ADD_DBUS_ADAPTOR(solidnettestdbusservice_SRCS ../solid/org.kde.Solid.Networking.xml
                         solidnettestdbusservice.h TestNetworkingService)

kde4_add_executable(solidnettestdbusservice NOGUI RUN_UNINSTALLED ${solidnettestdbusservice_SRCS})

target_link_libraries(solidnettestdbusservice ${QT_QTCORE_LIBRARY} ${QT_QTDBUS_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTTEST_LIBRARY})

########### install files ###############

endif(KDE4_BUILD_TESTS)


include(ECMMarkAsTest)

########### fakehardwaretest ###############

set(fakehardwaretest_SRCS fakehardwaretest.cpp )


add_executable(fakehardwaretest ${fakehardwaretest_SRCS})
add_test(fakehardwaretest fakehardwaretest)
ecm_mark_as_test(fakehardwaretest)

target_link_libraries(fakehardwaretest Solid_static Qt5::DBus ${QT_QTXML_LIBRARY} Qt5::Test)
# QT5 TODO Port solid to QFINDTESTDATA
set_target_properties(fakehardwaretest PROPERTIES COMPILE_FLAGS -DTEST_DATA="\\"${CMAKE_CURRENT_SOURCE_DIR}/../src/solid/backends/fakehw/fakecomputer.xml\\"")


########### halbasictest ###############

if(NOT WIN32 AND NOT APPLE)
   set(halbasictest_SRCS halbasictest.cpp )

   add_executable(halbasictest ${halbasictest_SRCS})

   target_link_libraries(halbasictest Solid_static Qt5::DBus ${QT_QTXML_LIBRARY} Qt5::Test)

   ecm_mark_as_test(halbasictest)
endif(NOT WIN32 AND NOT APPLE)

########### solidhwtest ###############

set(solidhwtest_SRCS
   solidhwtest.cpp )


add_executable(solidhwtest ${solidhwtest_SRCS})
add_test(solidhwtest solidhwtest)
ecm_mark_as_test(solidhwtest)
set_target_properties(solidhwtest PROPERTIES COMPILE_FLAGS -DSOLID_STATIC_DEFINE=1\ -DFAKE_COMPUTER_XML="\\"${CMAKE_CURRENT_SOURCE_DIR}/../src/solid/backends/fakehw/fakecomputer.xml\\"")

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../src/solid/backends/fakehw )

target_link_libraries(solidhwtest Qt5::DBus ${QT_QTXML_LIBRARY} Qt5::Test ${LIBS} Solid_static)

########### solidmttest ###############

set(solidmttest_SRCS
   solidmttest.cpp )

find_package(Qt5Transitional REQUIRED Concurrent)

add_executable(solidmttest ${solidmttest_SRCS})
add_test(solidmttest solidmttest)
ecm_mark_as_test(solidmttest)
set_target_properties(solidmttest PROPERTIES COMPILE_FLAGS -DSOLID_STATIC_DEFINE=1\ -DFAKE_COMPUTER_XML="\\"${CMAKE_CURRENT_SOURCE_DIR}/../src/solid/backends/fakehw/fakecomputer.xml\\"")

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../src/solid/backends/fakehw )

target_link_libraries(solidmttest Qt5::DBus ${QT_QTXML_LIBRARY} Qt5::Test ${LIBS} Solid_static Qt5::Concurrent)


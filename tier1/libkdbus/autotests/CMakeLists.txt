set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ../ )
include_directories( ${CMAKE_CURRENT_BINARY_DIR}/.. )
remove_definitions(-DQT_NO_CAST_FROM_ASCII)

find_package(Qt4 REQUIRED QtTest)
include(${QT_USE_FILE})

MACRO(KDBUS_UNIT_TESTS)
    FOREACH(_testname ${ARGN})
        add_executable(${_testname} ${_testname}.cpp)
        add_test(${_testname} ${_testname})
        target_link_libraries(${_testname} ${QT_QTCORE_LIBRARY} ${QT_QTDBUS_LIBRARY} ${QT_QTTEST_LIBRARY} kdbus)
    ENDFOREACH(_testname)
ENDMACRO(KDBUS_UNIT_TESTS)


########### next target ###############

KDBUS_UNIT_TESTS(
 kdbusinterprocesslocktest
 kdbusservicetest
)


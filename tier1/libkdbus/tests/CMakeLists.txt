set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ../ )
remove_definitions(-DQT_NO_CAST_FROM_ASCII)

MACRO(KDBUS_UNIT_TESTS)
    FOREACH(_testname ${ARGN})
        kde4_add_unit_test(${_testname} TESTNAME "kdbus-${_testname}" NOGUI ${_testname}.cpp)
        target_link_libraries(${_testname} ${QT_QTCORE_LIBRARY} ${QT_QTDBUS_LIBRARY} ${QT_QTTEST_LIBRARY} kdbus)
    ENDFOREACH(_testname)
ENDMACRO(KDBUS_UNIT_TESTS)


########### next target ###############

KDBUS_UNIT_TESTS(
 kdbusinterprocesslocktest
 kdbusservicetest
)


set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ../src )
include_directories( ${CMAKE_CURRENT_BINARY_DIR}/../src )
remove_definitions(-DQT_NO_CAST_FROM_ASCII)

find_package(Qt5Transitional REQUIRED Test)

MACRO(KDBUSADDONS_UNIT_TESTS)
    FOREACH(_testname ${ARGN})
        add_executable(${_testname} ${_testname}.cpp)
        add_test(${_testname} ${_testname})
        target_link_libraries(${_testname} ${QT_QTCORE_LIBRARY} ${QT_QTDBUS_LIBRARY} ${QT_QTTEST_LIBRARY} kdbusaddons)
    ENDFOREACH(_testname)
ENDMACRO(KDBUSADDONS_UNIT_TESTS)


########### next target ###############

KDBUSADDONS_UNIT_TESTS(
    kdbusinterprocesslocktest
    kdbusservicetest
)


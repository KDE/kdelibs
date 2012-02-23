set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/../src
   ${CMAKE_CURRENT_BINARY_DIR}/../src
)

find_package(Qt5Transitional REQUIRED Test Gui)
enable_testing()

MACRO(KPLOTTING_UNIT_TESTS)
    FOREACH(_testname ${ARGN})
        add_executable(${_testname} ${_testname}.cpp)
        add_test(${_testname} ${_testname})
        target_link_libraries(${_testname} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY} kplotting)
    ENDFOREACH(_testname)
ENDMACRO(KPLOTTING_UNIT_TESTS)


########### next target ###############

KPLOTTING_UNIT_TESTS(
    kplotaxistest
    kplotobjecttest
)

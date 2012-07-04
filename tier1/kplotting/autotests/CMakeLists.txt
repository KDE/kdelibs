include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/../src
   ${CMAKE_CURRENT_BINARY_DIR}/../src
)

find_package(Qt5Transitional REQUIRED Test Gui)

include(ECMMarkAsTest)

macro(KPLOTTING_UNIT_TESTS)
    foreach(_testname ${ARGN})
        add_executable(${_testname} ${_testname}.cpp)
        add_test(${_testname} ${_testname})
        target_link_libraries(${_testname} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY} ${QT_QTMAIN_LIBRARY} kplotting)
        ecm_mark_as_test(${_testname})
    endforeach()
endmacro()


########### next target ###############

kplotting_unit_tests(
    kplotpointtest
    kplotaxistest
    kplotobjecttest
    kplotwidgettest
)

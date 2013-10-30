
find_package(Qt5 5.2.0 CONFIG REQUIRED Test Widgets)

include(ECMMarkAsTest)

macro(KPLOTTING_UNIT_TESTS)
    foreach(_testname ${ARGN})
        add_executable(${_testname} ${_testname}.cpp)
        add_test(kplotting-${_testname} ${_testname})
        target_link_libraries(${_testname} Qt5::Test KPlotting)
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

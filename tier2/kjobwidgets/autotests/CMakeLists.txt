
find_package(Qt5 5.2.0 CONFIG REQUIRED Test)

include(ECMMarkAsTest)

macro(KJOBWIDGETS_TESTS)
    foreach(_testname ${ARGN})
        add_executable(${_testname} ${_testname}.cpp)
        add_test(kjobwidgets-${_testname} ${_testname})
        ecm_mark_as_test(${_testname})
        target_link_libraries(${_testname} Qt5::Test KF5::JobWidgets)
    endforeach(_testname)
endmacro()

kjobwidgets_tests(
)


find_package(Qt5 5.2.0 REQUIRED NO_MODULE COMPONENTS Xml Test)

include(ECMMarkAsTest)

macro(KNOTIFICATIONS_TESTS)
    foreach(_testname ${ARGN})
        add_executable(${_testname} ${_testname}.cpp)
        add_test(knotifications-${_testname} ${_testname})
        ecm_mark_as_test(${_testname})
        target_link_libraries(${_testname} Qt5::Test Qt5::Xml KNotifications)
    endforeach(_testname)
endmacro()

knotifications_tests(
)

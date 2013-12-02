
find_package(Qt5 5.2.0 CONFIG REQUIRED Test)

include(ECMMarkAsTest)

macro(KICONTHEMES_TESTS)
    foreach(_testname ${ARGN})
        add_executable(${_testname} ${_testname}.cpp)
        add_test(kiconthemes-${_testname} ${_testname})
        ecm_mark_as_test(${_testname})
        target_link_libraries(${_testname} Qt5::Test KIconThemes KF5::WidgetsAddons)
    endforeach(_testname)
endmacro()

kiconthemes_tests(
  kiconloader_unittest
)

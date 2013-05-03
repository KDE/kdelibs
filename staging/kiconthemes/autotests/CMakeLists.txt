
find_package(Qt5 REQUIRED NO_MODULE COMPONENTS Xml Test)

include(ECMMarkAsTest)

macro(KICONTHEMES_TESTS)
    foreach(_testname ${ARGN})
        add_executable(${_testname} ${_testname}.cpp)
        add_test(kiconthemes-${_testname} ${_testname})
        ecm_mark_as_test(${_testname})
        target_link_libraries(${_testname} Qt5::Test Qt5::Xml KIconThemes)
    endforeach(_testname)
endmacro()

kiconthemes_tests(
  kiconloader_unittest
)


include(ECMMarkAsTest)

find_package(Qt5 REQUIRED NO_MODULE COMPONENTS Xml Test)

macro(kcompletion_executable_tests)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    target_link_libraries(${_testname} Qt5::Test Qt5::Xml KCompletion)
    ecm_mark_as_test(${_testname})
  endforeach(_testname)
endmacro()

kcompletion_executable_tests(
)


include(ECMMarkAsTest)

find_package(Qt5 5.2.0 REQUIRED NO_MODULE COMPONENTS Test)

macro(kde4attic_executable_tests)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    target_link_libraries(${_testname} Qt5::Test KI18n KDE4Attic)
    ecm_mark_as_test(${_testname})
  endforeach(_testname)
endmacro()

kde4attic_executable_tests(
  kanimatedbuttontest
  kcharselecttest
  kdatepicktest
  kdatetabletest
  kdatetimeedittest
  kdatetimewidgettest
  kdatewidgettest
)

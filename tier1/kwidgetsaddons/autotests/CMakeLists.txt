
include(ECMMarkAsTest)

find_package(Qt5 5.2.0 REQUIRED NO_MODULE COMPONENTS Test)

macro(kwidgetsaddons_unit_tests)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    target_link_libraries(${_testname} Qt5::Test KWidgetsAddons)
    add_test(kwidgetsaddons-${_testname} ${_testname})
    ecm_mark_as_test(${_testname})
  endforeach(_testname)
endmacro()

kwidgetsaddons_unit_tests(
  kcolorbuttontest
  kdatecomboboxtest
  kdatetimeedittest
  kdualactiontest
  kselectaction_unittest
  ktimecomboboxtest
)

find_package(Qt5Transitional REQUIRED MODULE COMPONENTS Test Widgets)

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include(ECMMarkAsTest)

macro(kguiaddons_unit_tests)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    add_test(kconfig-${_testname} ${_testname})
    target_link_libraries(${_testname} kguiaddons Qt5::Test Qt5::Widgets)
    ecm_mark_as_test(${_testname})
  endforeach()
endmacro()

kguiaddons_unit_tests(
  kwordwraptest
  kcolorutilstest
)

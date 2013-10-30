find_package(Qt5 5.2.0 CONFIG REQUIRED Test)

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include(ECMMarkAsTest)

macro(kguiaddons_unit_tests)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    add_test(kguiaddons-${_testname} ${_testname})
    target_link_libraries(${_testname} KGuiAddons Qt5::Test)
    ecm_mark_as_test(${_testname})
  endforeach()
endmacro()

kguiaddons_unit_tests(
  kwordwraptest
  kcolorutilstest
  kiconutilstest
)

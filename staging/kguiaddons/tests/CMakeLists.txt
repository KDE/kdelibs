remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_TO_ASCII)

include(ECMMarkAsTest)

find_package(Qt5 5.2.0 REQUIRED NO_MODULE COMPONENTS Xml Test)

macro(kguiaddons_executable_tests)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    target_link_libraries(${_testname} KGuiAddons Qt5::Widgets)
    ecm_mark_as_test(${_testname})
  endforeach(_testname)
endmacro()

kguiaddons_executable_tests(
  kcolorcollectiontest
  kmodifierkeyinfotest
  openurltest
)

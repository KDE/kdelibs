
include(ECMMarkAsTest)

macro(kwidgetsaddons_executable_tests)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    target_link_libraries(${_testname} Qt5::Widgets ${QT_QTTEST_LIBRARY} ${QT_QTXML_LIBRARY} KWidgetsAddons)
    ecm_mark_as_test(${_testname})
  endforeach(_testname)
endmacro()

kwidgetsaddons_executable_tests(
  kledtest
  kseparatortest
)


include(ECMMarkAsTest)

macro(kwidgetsaddons_executable_tests)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    target_link_libraries(${_testname} Qt5::Widgets ${QT_QTTEST_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTMAIN_LIBRARY} kwidgetsaddons)
    ecm_mark_as_test(${_testname})
  endforeach(_testname)
endmacro()

include_directories(
  "${CMAKE_CURRENT_SOURCE_DIR}/../src"
  "${CMAKE_CURRENT_BINARY_DIR}/../src"
)

kwidgetsaddons_executable_tests(
  kledtest
  kseparatortest
)

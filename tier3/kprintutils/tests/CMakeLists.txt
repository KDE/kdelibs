include(ECMMarkAsTest)

find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test PrintSupport Widgets)

macro(kprintutils_executable_tests)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    target_link_libraries(${_testname} Qt5::Test KPrintUtils)
    ecm_mark_as_test(${_testname})
  endforeach()
endmacro()

kprintutils_executable_tests(
  kprintpreview_test
)

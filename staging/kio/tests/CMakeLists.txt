set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include(ECMMarkAsTest)

macro(KIOWIDGETS_EXECUTABLE_TESTS)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    add_test(kiowidgets-${_testname} ${_testname})
    target_link_libraries(${_testname} KIOCore KIOWidgets Qt5::Test)
    ecm_mark_as_test(${_testname})
  endforeach()
endmacro(KIOWIDGETS_EXECUTABLE_TESTS)

KIOWIDGETS_EXECUTABLE_TESTS(
  kruntest
)

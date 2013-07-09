
include(ECMMarkAsTest)

macro(ktextwidgets_executable_tests)
   foreach(_testname ${ARGN})
      add_executable(${_testname} ${_testname}.cpp)
      target_link_libraries(${_testname} Qt5::Widgets Qt5::Test SonnetUi KTextWidgets)
      ecm_mark_as_test(${_testname})
   endforeach()
endmacro()

ktextwidgets_executable_tests(
  ktextedittest
)

include(ECMMarkAsTest)

macro(ktextwidgets_unit_tests)
   foreach(_testname ${ARGN})
      add_executable(${_testname} ${_testname}.cpp)
      add_test(ktextwidgets-${_testname} ${_testname})
      target_link_libraries(${_testname} Qt5::Widgets Qt5::Test SonnetUi KTextWidgets)
      ecm_mark_as_test(${_testname})
   endforeach()
endmacro()

ktextwidgets_unit_tests(
  kfindtest
  kreplacetest
  ktextedit_unittest
)

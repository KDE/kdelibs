include(ECMMarkAsTest)

find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test)

macro(ktextwidgets_unit_tests)
   foreach(_testname ${ARGN})
      add_executable(${_testname} ${_testname}.cpp)
      add_test(ktextwidgets-${_testname} ${_testname})
      target_link_libraries(${_testname} Qt5::Test KF5::TextWidgets KF5::KConfigWidgets)
      ecm_mark_as_test(${_testname})
   endforeach()
endmacro()

ktextwidgets_unit_tests(
  kfindtest
  kreplacetest
  krichtextedittest
  ktextedit_unittest
)

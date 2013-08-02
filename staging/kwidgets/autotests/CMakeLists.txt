include(ECMMarkAsTest)

macro(kwidgets_unit_tests)
   foreach(_testname ${ARGN})
      add_executable(${_testname} ${_testname}.cpp)
      add_test(kwidgets-${_testname} ${_testname})
      target_link_libraries(${_testname} KWidgets KConfigGui Qt5::Widgets Qt5::Test Qt5::Xml)
      ecm_mark_as_test(${_testname})
   endforeach()
endmacro()

kwidgets_unit_tests(
    kstandardactiontest
)

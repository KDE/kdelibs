include(ECMMarkAsTest)

macro(kdeclarative_executable_tests)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    target_link_libraries(${_testname} Qt5::Quick KDeclarative KI18n KConfigCore)
    ecm_mark_as_test(${_testname})
  endforeach(_testname)
endmacro()

kdeclarative_executable_tests(
  kdeclarativetest
)

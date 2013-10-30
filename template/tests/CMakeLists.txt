find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test)

include(ECMMarkAsTest)

macro(foobar_executable_tests)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    target_link_libraries(${_testname} Qt5::Test FooBar)
    ecm_mark_as_test(${_testname})
  endforeach(_testname)
endmacro()

#foobar_executable_tests(
#  anothertest
#)

include(ECMAddTests)

find_package(Qt5Test REQUIRED)
macro(kemoticons_unit_tests)
   foreach(_testname ${ARGN})
      add_executable(${_testname} ${_testname}.cpp)
      add_test(kemoticons-${_testname} ${_testname})
      target_link_libraries(${_testname} Qt5::Test KEmoticons)
      ecm_mark_as_test(${_testname})
   endforeach()
endmacro()

kemoticons_unit_tests(kemoticontest)


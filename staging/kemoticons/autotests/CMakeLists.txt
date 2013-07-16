
macro(kemoticons_unit_tests)
   foreach(_testname ${ARGN})
      add_executable(${_testname} ${_testname}.cpp)
      add_test(kemoticons-${_testname} ${_testname})
      target_link_libraries(${_testname} ${KDE4_KIO_LIBS} Qt5::Test ${KCoreAddons_LIBRARIES} KEmoticons)
      ecm_mark_as_test(${_testname})
   endforeach()
endmacro()

kemoticons_unit_tests(kemoticontest)


set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include(ECMMarkAsTest)

MACRO(KFILE_UNIT_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_add_unit_test(${_testname} TESTNAME kfile-${_testname} ${_testname}.cpp)
               set_property(TARGET ${_testname} APPEND_STRING PROPERTY COMPILE_FLAGS " -DTEST_DATA=\"\\\"${CMAKE_SOURCE_DIR}/kfile/tests/fakecomputer.xml\\\"\"")
               target_link_libraries(${_testname} ${KDE4_KFILE_LIBS} ${KDE4_KIO_LIBS} Qt5::Test Qt5::Xml Solid)
       ENDFOREACH(_testname)
ENDMACRO(KFILE_UNIT_TESTS)
MACRO(KFILE_EXECUTABLE_TESTS)
       FOREACH(_testname ${ARGN})
               add_executable(${_testname} ${_testname}.cpp)
               ecm_mark_as_test(${_testname})
               target_link_libraries(${_testname} ${KDE4_KFILE_LIBS} Qt5::Test)
       ENDFOREACH(_testname)
ENDMACRO(KFILE_EXECUTABLE_TESTS)

KFILE_EXECUTABLE_TESTS(
  kdirselectdialogtest
  kfiletreeviewtest
)

KFILE_UNIT_TESTS(
  kurlnavigatortest
  kdiroperatortest
  knewfilemenutest
)

KFILE_UNIT_TESTS(
  kfileplacesmodeltest
)


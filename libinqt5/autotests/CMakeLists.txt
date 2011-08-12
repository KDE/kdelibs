set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/../src
   ${CMAKE_CURRENT_BINARY_DIR}/../src
)

MACRO(INQT5_UNIT_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_add_unit_test(${_testname} TESTNAME "inqt5-${_testname}" NOGUI ${_testname}.cpp)
               target_link_libraries(${_testname} inqt5 ${QT_QTTEST_LIBRARY})
       ENDFOREACH(_testname)
ENDMACRO(INQT5_UNIT_TESTS)

INQT5_UNIT_TESTS(
  tst_qcoredesktopservices
)


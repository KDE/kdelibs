enable_testing()
set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/../src
   ${CMAKE_CURRENT_BINARY_DIR}/../src
)

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

MACRO(QT5_STAGING_UNIT_TESTS)
       FOREACH(_testname ${ARGN})
               add_executable(${_testname} ${_testname}.cpp)
               add_test(qt5staging-${_testname} ${_testname})
               target_link_libraries(${_testname} kdeqt5staging ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY} ${INQT5_LIBRARY})
               set_target_properties(${_testname} PROPERTIES COMPILE_FLAGS -DKDESRCDIR="\\"${CMAKE_CURRENT_SOURCE_DIR}/\\"")
       ENDFOREACH()
ENDMACRO()

QT5_STAGING_UNIT_TESTS(
  tst_qurlpathinfo
  tst_qsavefile
)

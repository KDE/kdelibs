enable_testing()
set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/../src
   ${CMAKE_CURRENT_BINARY_DIR}/../src
)

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

macro(QT5_STAGING_UNIT_TESTS)
   foreach(_testname ${ARGN})
      add_executable(${_testname} ${_testname}.cpp)
      add_test(qt5staging-${_testname} ${_testname})
      target_link_libraries(${_testname} kdeqt5staging ${QT_QTTEST_LIBRARY})
   endforeach()
endmacro()

qt5_staging_unit_tests(
  tst_qurlpathinfo
)

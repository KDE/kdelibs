enable_testing()
set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/../src/core
   ${CMAKE_CURRENT_BINARY_DIR}/../src/core
)

MACRO(KARCHIVE_UNIT_TESTS)
       FOREACH(_testname ${ARGN})
               add_executable(${_testname} ${_testname}.cpp) # TODO NOGUI
               add_test(kconfig-${_testname} ${_testname})
               target_link_libraries(${_testname} kconfigcore ${QT_QTTEST_LIBRARY} ${QT_QTCORE_LIBRARY} ${INQT5_LIBRARY})
       ENDFOREACH(_testname)
ENDMACRO(KARCHIVE_UNIT_TESTS)

KARCHIVE_UNIT_TESTS(
  kconfigtest
)

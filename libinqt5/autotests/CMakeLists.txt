set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/../src
   ${CMAKE_CURRENT_BINARY_DIR}/../src
)

# TODO: Remove these
remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_USE_FAST_CONCATENATION)
remove_definitions(-DQT_USE_FAST_OPERATOR_PLUS)


MACRO(INQT5_UNIT_TESTS)
       FOREACH(_testname ${ARGN})
               add_executable(${_testname} ${_testname}.cpp)
               add_test(${_testname} "inqt5-${_testname}")
               target_link_libraries(${_testname} inqt5 ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY})
               set_target_properties(${_testname} PROPERTIES COMPILE_FLAGS -DKDESRCDIR="\\"${CMAKE_CURRENT_SOURCE_DIR}/\\"")
       ENDFOREACH(_testname)
ENDMACRO(INQT5_UNIT_TESTS)

INQT5_UNIT_TESTS(
  tst_qstandardpaths
)


set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/core
  ${CMAKE_CURRENT_BINARY_DIR}/../src/core

  ${CMAKE_CURRENT_SOURCE_DIR}/../../../kdecore/config
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../kdecore/io
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../kdecore/util
  ${CMAKE_CURRENT_BINARY_DIR}/../../../kdecore
  ${CMAKE_CURRENT_BINARY_DIR}/../../../staging/kcoreaddons/src
)

########### unittests ###############

MACRO(SONNET_UNIT_TESTS)
       FOREACH(_testname ${ARGN})
               add_executable(${_testname} ${_testname}.cpp)
               add_test(sonnet-${_testname} ${_testname})
               target_link_libraries(${_testname} ${KDE4_KDECORE_LIBS} sonnetcore ${QT_QTTEST_LIBRARY} ${QT_QTXML_LIBRARY} ${KDEWIN_LIBRARIES})
       ENDFOREACH(_testname)
ENDMACRO(SONNET_UNIT_TESTS)

SONNET_UNIT_TESTS(
 test_filter
 test_core
)


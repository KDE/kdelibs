enable_testing()
set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_KEYWORDS)

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/../src/core
   ${CMAKE_CURRENT_BINARY_DIR}/../src/core
   ${CMAKE_CURRENT_SOURCE_DIR}/../src/gui
   ${CMAKE_CURRENT_BINARY_DIR}/../src/gui
)

MACRO(KCONFIGCORE_UNIT_TESTS)
       FOREACH(_testname ${ARGN})
               add_executable(${_testname} ${_testname}.cpp) # TODO NOGUI
               add_test(kconfigcore-${_testname} ${_testname})
               target_link_libraries(${_testname} kconfigcore ${QT_QTTEST_LIBRARY} ${QT_QTCORE_LIBRARY} ${INQT5_LIBRARY})
       ENDFOREACH(_testname)
ENDMACRO(KCONFIGCORE_UNIT_TESTS)

MACRO(KCONFIGGUI_UNIT_TESTS)
       FOREACH(_testname ${ARGN})
               add_executable(${_testname} ${_testname}.cpp) # TODO NOGUI
               add_test(kconfiggui-${_testname} ${_testname})
               target_link_libraries(${_testname} kconfigcore kconfiggui ${QT_QTTEST_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${INQT5_LIBRARY})
       ENDFOREACH(_testname)
ENDMACRO(KCONFIGGUI_UNIT_TESTS)

KCONFIGCORE_UNIT_TESTS(
  kconfigtest
)

KCONFIGGUI_UNIT_TESTS(
  kconfigguitest
  kconfigskeletontest
)

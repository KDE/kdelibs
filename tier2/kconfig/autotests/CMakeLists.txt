set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_KEYWORDS)

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/../src/core
   ${CMAKE_CURRENT_BINARY_DIR}/../src/core
   ${CMAKE_CURRENT_SOURCE_DIR}/../src/gui
   ${CMAKE_CURRENT_BINARY_DIR}/../src/gui
)

macro(KCONFIGCORE_UNIT_TESTS)
   foreach(_testname ${ARGN})
      add_executable(${_testname} ${_testname}.cpp) # TODO NOGUI
      add_test(kconfigcore-${_testname} ${_testname})
      target_link_libraries(${_testname} kconfigcore ${QT_QTTEST_LIBRARY} ${QT_QTCORE_LIBRARY} ${inqt5_LIBRARY})
   endforeach()
endmacro()

macro(KCONFIGGUI_UNIT_TESTS)
   foreach(_testname ${ARGN})
      add_executable(${_testname} ${_testname}.cpp) # TODO NOGUI
      add_test(kconfiggui-${_testname} ${_testname})
      target_link_libraries(${_testname} kconfigcore kconfiggui ${QT_QTTEST_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${inqt5_LIBRARY})
   endforeach()
endmacro()

kconfigcore_unit_tests(
   kconfigtest
)

kconfiggui_unit_tests(
   kconfigguitest
   kconfigskeletontest
)

add_subdirectory(kconfig_compiler)


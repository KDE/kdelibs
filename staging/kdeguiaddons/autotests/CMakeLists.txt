find_package(Qt5Transitional REQUIRED Test Gui)
enable_testing()

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

remove_definitions(-DQT_NO_CAST_FROM_ASCII)
add_definitions(-DQT_GUI_LIB)

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/../src
   ${CMAKE_CURRENT_BINARY_DIR}/../src
)

macro(kguiaddons_unit_tests)
       foreach(_testname ${ARGN})
               add_executable(${_testname} ${_testname}.cpp)
               add_test(kconfig-${_testname} ${_testname})
               target_link_libraries(${_testname} kdeguiaddons ${QT_QTTEST_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${inqt5_LIBRARY})
       endforeach()
endmacro()

kguiaddons_unit_tests(
  kwordwraptest
)

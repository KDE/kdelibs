find_package(Qt5Transitional REQUIRED MODULE COMPONENTS Test Gui)

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../src
  ${CMAKE_CURRENT_BINARY_DIR}/../src
)

include(ECMMarkAsTest)

macro(kguiaddons_unit_tests)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    add_test(kconfig-${_testname} ${_testname})
    target_link_libraries(${_testname} kguiaddons ${QT_QTTEST_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTMAIN_LIBRARY})
    ecm_mark_as_test(${_testname})
  endforeach()
endmacro()

kguiaddons_unit_tests(
  kwordwraptest
  kcolorutilstest
)



macro(kwidgetsaddons_executable_tests)
       foreach(_testname ${ARGN})
               add_executable(${_testname} ${_testname}.cpp)
               add_test(kwidgetsaddons-${_testname} ${_testname})
               target_link_libraries(${_testname} ${QT_QTGUI_LIBRARIES} ${QT_QTTEST_LIBRARY} ${QT_QTXML_LIBRARY} kwidgetsaddons)
       endforeach(_testname)
endmacro()

include_directories(
  "${CMAKE_CURRENT_SOURCE_DIR}/../src"
  "${CMAKE_CURRENT_BINARY_DIR}/../src"
)

kwidgetsaddons_executable_tests(
  kledtest
)
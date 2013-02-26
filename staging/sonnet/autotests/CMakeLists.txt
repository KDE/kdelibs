remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/core
  ${CMAKE_CURRENT_BINARY_DIR}/../src/core

  ${KCoreAddons_INCLUDE_DIRS}
)

include(ECMMarkAsTest)

########### unittests ###############

macro(SONNET_UNIT_TESTS)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    add_test(sonnet-${_testname} ${_testname})
    target_link_libraries(${_testname} ${KDE4_KDECORE_LIBS} sonnetcore ${QT_QTTEST_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTMAIN_LIBRARY} ${KDEWIN_LIBRARIES})
    ecm_mark_as_test(${_testname})
  endforeach()
endmacro(SONNET_UNIT_TESTS)

sonnet_unit_tests(
 test_filter
 test_core
)


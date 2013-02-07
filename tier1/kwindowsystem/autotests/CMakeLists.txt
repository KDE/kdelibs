remove_definitions(-DQT_NO_KEYWORDS)
remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_TO_ASCII)

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/../src
   ${CMAKE_CURRENT_BINARY_DIR}/../src
)

include(ECMMarkAsTest)

macro(KWINDOWSYSTEM_UNIT_TESTS)
   foreach(_testname ${ARGN})
      add_executable(${_testname} ${_testname}.cpp)
      add_test(kwindowsystem-${_testname} ${_testname})
      # TODO: QT5 remove KDESRCDIR, QFINDTESTDATA doesn't need it
      target_link_libraries(${_testname} kwindowsystem ${QT_QTTEST_LIBRARY} ${QT_QTNETWORK_LIBRARY} Qt5::Widgets ${kdeqt5staging_LIBRARIES} ${QT_QTMAIN_LIBRARY})
      ecm_mark_as_test(${_testname})
   endforeach(_testname)
endmacro(KWINDOWSYSTEM_UNIT_TESTS)

kwindowsystem_unit_tests(
   kstartupinfo_unittest
   kxmessages_unittest
)


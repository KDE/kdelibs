remove_definitions(-DQT_NO_KEYWORDS)
remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_TO_ASCII)

include(ECMMarkAsTest)

find_package(Qt5 REQUIRED NO_MODULE COMPONENTS Test Network)

macro(KWINDOWSYSTEM_UNIT_TESTS)
   foreach(_testname ${ARGN})
      add_executable(${_testname} ${_testname}.cpp)
      add_test(kwindowsystem-${_testname} ${_testname})
      target_link_libraries(${_testname} KWindowSystem Qt5::Test Qt5::DBus Qt5::Network Qt5::Widgets Qt5::X11Extras )
      ecm_mark_as_test(${_testname})
   endforeach(_testname)
endmacro(KWINDOWSYSTEM_UNIT_TESTS)
macro(KWINDOWSYSTEM_EXECUTABLE_TESTS)
   foreach(_testname ${ARGN})
      add_executable(${_testname} ${_testname}.cpp)
      target_link_libraries(${_testname} KWindowSystem Qt5::Test Qt5::X11Extras)
      ecm_mark_as_test(${_testname})
   endforeach()
endmacro()

kwindowsystem_unit_tests(
   kglobalshortcuttest
   kstartupinfo_unittest
   kxmessages_unittest
)

kwindowsystem_executable_tests(
   fixx11h_test
   fixx11h_test2
)

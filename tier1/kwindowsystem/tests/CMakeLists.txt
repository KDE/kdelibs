remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_TO_ASCII)

include(ECMMarkAsTest)

find_package(Qt5 5.2.0 CONFIG REQUIRED Test)

macro(kwindowsystem_executable_tests)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    target_link_libraries(${_testname} Qt5::Test Qt5::Widgets KF5::WindowSystem)
    ecm_mark_as_test(${_testname})
  endforeach(_testname)
endmacro()

kwindowsystem_executable_tests(
  setmainwindowtest
  createpixmapfromhandletest
  blurbehindtest
)


include(ECMMarkAsTest)

if (X11_FOUND)
find_package(Qt5X11Extras 5.2.0 REQUIRED NO_MODULE)

macro(kwindowsystem_executable_tests)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    target_link_libraries(${_testname} ${X11_X11_LIB} KWindowSystem)
    ecm_mark_as_test(${_testname})
  endforeach(_testname)
endmacro()

kwindowsystem_executable_tests(
  kxerrorhandlertest
)
endif (X11_FOUND)

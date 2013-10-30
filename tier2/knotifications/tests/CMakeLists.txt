
include(ECMMarkAsTest)
find_package(ECM 0.0.9 REQUIRED NO_MODULE)

find_package(Qt5 5.2.0 CONFIG REQUIRED Test)

macro(knotifications_executable_tests)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    target_link_libraries(${_testname} Qt5::Test KNotifications)
    ecm_mark_as_test(${_testname})
  endforeach(_testname)
endmacro()

knotifications_executable_tests(
    knotificationrestrictionstest
    kstatusnotifieritemtest
)

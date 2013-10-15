
include(ECMMarkAsTest)

find_package(Qt5 5.2.0 REQUIRED NO_MODULE COMPONENTS Test)

macro(kde4attic_executable_tests)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    target_link_libraries(${_testname} Qt5::Test KF5::KI18n KDE4Attic)
    ecm_mark_as_test(${_testname})
  endforeach(_testname)
endmacro()

kde4attic_executable_tests(
  kcharselecttest
  kdatepicktest
  kdatetimeedittestapp
  kdatetimewidgettest
  kdatewidgettest
)

add_executable(kdatetabletest kdatetabletest.cpp ../src/kdatetable.cpp)
target_include_directories(kdatetabletest PRIVATE ../src)
target_link_libraries(kdatetabletest Qt5::Widgets)
ecm_mark_as_test(kdatetabletest)


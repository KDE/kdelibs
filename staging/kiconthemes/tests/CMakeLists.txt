
include(ECMMarkAsTest)

find_package(Qt5 5.2.0 REQUIRED NO_MODULE COMPONENTS Xml Test)

macro(kiconthemes_executable_tests)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    target_link_libraries(${_testname} Qt5::Test Qt5::Xml KIconThemes kdeui)
    ecm_mark_as_test(${_testname})
  endforeach(_testname)
endmacro()

kiconthemes_executable_tests(
  kiconloadertest
  kiconeffecttest
)

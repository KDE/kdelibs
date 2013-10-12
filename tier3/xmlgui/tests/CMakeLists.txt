
include(ECMMarkAsTest)

find_package(Qt5 5.2.0 REQUIRED NO_MODULE COMPONENTS Test)

macro(xmlgui_executable_tests)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    target_link_libraries(${_testname} Qt5::Test KF5::KWidgetsAddons KF5::KI18n XmlGui)
    ecm_mark_as_test(${_testname})
  endforeach(_testname)
endmacro()

xmlgui_executable_tests(
  kbugreporttest
  kmainwindowrestoretest
  kmainwindowtest
  krulertest
  ktoolbartest
  kxmlguitest
  kxmlguiwindowtest
  kwindowtest
)

add_subdirectory(krichtexteditor)

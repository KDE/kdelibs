set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include(ECMMarkAsTest)

macro(KIOWIDGETS_EXECUTABLE_TESTS)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    target_link_libraries(${_testname} KIOCore KIOWidgets KIOFileWidgets Qt5::Test KF5::KWidgetsAddons KF5::KIconThemes XmlGui)
    ecm_mark_as_test(${_testname})
  endforeach()
endmacro(KIOWIDGETS_EXECUTABLE_TESTS)

KIOWIDGETS_EXECUTABLE_TESTS(
  getalltest
  kruntest
  kioslavetest
  kopenwithtest
  kdirmodeltest_gui
  kdirlistertest_gui
  previewtest
  kurlrequestertest_gui
  kfiletreeviewtest
  kpropertiesdialogtest
  kmountpoint_debug
)

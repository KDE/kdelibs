
find_package(Qt5 5.2.0 CONFIG REQUIRED Xml Test)

remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_TO_ASCII)

include(ECMMarkAsTest)

macro(XMLGUI_TESTS)
    foreach(_testname ${ARGN})
        add_executable(${_testname} ${_testname}.cpp)
        add_test(kxmlgui-${_testname} ${_testname})
        ecm_mark_as_test(${_testname})
        target_link_libraries(${_testname} Qt5::Test KF5::I18n KF5::IconThemes KF5::XmlGui)
    endforeach(_testname)
endmacro()

xmlgui_tests(
   kactioncategorytest
   kactioncollectiontest
   kmainwindow_unittest
   ktoolbar_unittest
   kxmlgui_unittest
)

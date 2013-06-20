
find_package(Qt5 5.2.0 REQUIRED NO_MODULE COMPONENTS Xml Test)

remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_TO_ASCII)

include(ECMMarkAsTest)

macro(XMLGUI_TESTS)
    foreach(_testname ${ARGN})
        add_executable(${_testname} ${_testname}.cpp)
        add_test(xmlgui-${_testname} ${_testname})
        ecm_mark_as_test(${_testname})
        target_link_libraries(${_testname} Qt5::Test Qt5::Xml XmlGui)
    endforeach(_testname)
endmacro()

xmlgui_tests(
   kglobalshortcuttest
)

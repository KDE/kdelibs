include(ECMMarkAsTest)

include_directories(
   ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
)

set(CONFIGFILE "${CMAKE_CURRENT_SOURCE_DIR}/kdeplatformtheme_kdeglobals")
configure_file(kdeplatformtheme_config.h.in ${CMAKE_CURRENT_BINARY_DIR}/kdeplatformtheme_config.h)

remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_KEYWORDS)

macro(FRAMEWORKINTEGRATION_TESTS _testname)
    add_executable(${_testname} ${_testname}.cpp ${ARGN})
    add_test(frameworkintegration-${_testname} ${_testname})
    ecm_mark_as_test(${_testname})
    target_link_libraries(${_testname} Qt5::Test Qt5::DBus KConfigWidgets KConfigCore KIconThemes)
endmacro()

frameworkintegration_tests(
  kdeplatformtheme_unittest
  ../src/platformtheme/kdeplatformtheme.cpp
  ../src/platformtheme/kfontsettingsdata.cpp
)

frameworkintegration_tests(
  kfontsettingsdata_unittest
  ../src/platformtheme/kfontsettingsdata.cpp
)
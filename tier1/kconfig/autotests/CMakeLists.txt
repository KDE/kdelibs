
remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_KEYWORDS)

include(ECMMarkAsTest)

find_package(Qt5 5.2.0 REQUIRED NO_MODULE COMPONENTS Concurrent Test Widgets)

macro(KCONFIGCORE_UNIT_TESTS)
   foreach(_testname ${ARGN})
      add_executable(${_testname} ${_testname}.cpp) # TODO NOGUI
      add_test(kconfigcore-${_testname} ${_testname})
      target_link_libraries(${_testname} KConfigCore Qt5::Test Qt5::Concurrent)
      ecm_mark_as_test(${_testname})
   endforeach()
endmacro()

macro(KCONFIGGUI_UNIT_TESTS)
   foreach(_testname ${ARGN})
      add_executable(${_testname} ${_testname}.cpp)
      add_test(kconfiggui-${_testname} ${_testname})
      target_link_libraries(${_testname} KConfigGui Qt5::Test)
      ecm_mark_as_test(${_testname})
   endforeach()
endmacro()

kconfigcore_unit_tests(
   kconfignokdehometest
   kconfigtest
   kdesktopfiletest
   ksharedconfigtest
   test_kconf_update
)

target_include_directories(test_kconf_update PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/../src/kconf_update)

# compile KEntryMap into the test since it's not exported
set(kentrymaptest_SRCS kentrymaptest.cpp ../src/core/kconfigdata.cpp)
add_executable(kentrymaptest ${kentrymaptest_SRCS})
add_test(kconfigcore-kentrymaptest kentrymaptest)
target_link_libraries(kentrymaptest KConfigCore Qt5::Test)
ecm_mark_as_test(kentrymaptest)

# compile KConfigUtils into the test since it's not exported
set(test_kconfigutils_SRCS test_kconfigutils ../src/kconf_update/kconfigutils.cpp)
add_executable(test_kconfigutils ${test_kconfigutils_SRCS})
add_test(kconfigcore-test_kconfigutils test_kconfigutils)
target_link_libraries(test_kconfigutils KConfigCore Qt5::Test)
target_include_directories(test_kconfigutils PRIVATE ../src/kconf_update)
ecm_mark_as_test(test_kconfigutils)

kconfiggui_unit_tests(
   kconfigguitest
   kconfigloadertest
   kconfigskeletontest
   kstandardshortcuttest
)

add_subdirectory(kconfig_compiler)


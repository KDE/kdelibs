
remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_KEYWORDS)

include(ECMMarkAsTest)

find_package(Qt5Transitional REQUIRED Concurrent)

macro(KCONFIGCORE_UNIT_TESTS)
   foreach(_testname ${ARGN})
      add_executable(${_testname} ${_testname}.cpp) # TODO NOGUI
      add_test(kconfigcore-${_testname} ${_testname})
      target_link_libraries(${_testname} KConfigCore ${QT_QTTEST_LIBRARY} Qt5::Concurrent)
      ecm_mark_as_test(${_testname})
   endforeach()
endmacro()

find_package(Qt5Transitional REQUIRED Concurrent)

macro(KCONFIGGUI_UNIT_TESTS)
   foreach(_testname ${ARGN})
      add_executable(${_testname} ${_testname}.cpp)
      add_test(kconfiggui-${_testname} ${_testname})
      target_link_libraries(${_testname} KConfigCore KConfigGui ${QT_QTTEST_LIBRARY} Qt5::Widgets)
      ecm_mark_as_test(${_testname})
   endforeach()
endmacro()

kconfigcore_unit_tests(
   kconfigtest
   kdesktopfiletest
   ksharedconfigtest
   kentrymaptest
)

kconfiggui_unit_tests(
   kconfigguitest
   kconfigskeletontest
)
if(WIN32)
    target_link_libraries(kconfigguitest ${KDEWIN_LIBRARIES})
endif()

add_subdirectory(kconfig_compiler)


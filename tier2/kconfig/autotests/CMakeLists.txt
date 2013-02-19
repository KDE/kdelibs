
remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_KEYWORDS)

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/../src/core
   ${CMAKE_CURRENT_BINARY_DIR}/../src/core
   ${CMAKE_CURRENT_SOURCE_DIR}/../src/gui
   ${CMAKE_CURRENT_BINARY_DIR}/../src/gui
)

include(ECMMarkAsTest)

find_package(Qt5Transitional REQUIRED Concurrent)

macro(KCONFIGCORE_UNIT_TESTS)
   foreach(_testname ${ARGN})
      add_executable(${_testname} ${_testname}.cpp) # TODO NOGUI
      add_test(kconfigcore-${_testname} ${_testname})
      target_link_libraries(${_testname} kconfigcore ${QT_QTTEST_LIBRARY} ${QT_QTMAIN_LIBRARY} Qt5::Concurrent)
      ecm_mark_as_test(${_testname})
   endforeach()
endmacro()

macro(KCONFIGGUI_UNIT_TESTS)
   foreach(_testname ${ARGN})
      add_executable(${_testname} ${_testname}.cpp)
      add_test(kconfiggui-${_testname} ${_testname})
      target_link_libraries(${_testname} kconfigcore kconfiggui ${QT_QTTEST_LIBRARY} Qt5::Widgets ${QT_QTMAIN_LIBRARY})
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


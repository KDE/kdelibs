
include(ECMMarkAsTest)

include_directories("${CMAKE_CURRENT_BINARY_DIR}/../../..")

macro(KDE4SUPPORT_EXECUTABLE_TESTS)
    foreach(_testname ${ARGN})
        add_executable(${_testname} ${_testname}.cpp)
        target_link_libraries(${_testname} kde4support Qt5::Core)
        ecm_mark_as_test(${_testname})
    endforeach()
endmacro()

macro(KDE4SUPPORT_UNIT_TESTS)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    add_test(kde4support-${_testname} ${_testname})
    target_link_libraries(${_testname} kde4support Qt5::Test Qt5::Network kdecore ${KCoreAddons_LIBRARIES} ki18n kdeui)
    ecm_mark_as_test(${_testname})
  endforeach(_testname)
endmacro(KDE4SUPPORT_UNIT_TESTS)

kde4support_unit_tests(
  globalcleanuptest
  kdebugtest
  kdialog_unittest
  kfadewidgeteffecttest
  kconfigafterkglobaltest1
  kconfigafterkglobaltest2
  kdebug_unittest
  kdebug_qcoreapptest
  kmd5benchmark
  ktempdirtest
  ktemporaryfiletest
  kuniqueapptest
  kurlmimetest
  klibloadertest
  ksavefiletest
  kconfigcompattest
  kglobalstatictest
  kmimetypetest
  kmimetype_nomimetypes
  kstandarddirstest
  kasciitest
  kglobaltest
  klockfiletest
)


# Helper for klockfiletest
kde4support_executable_tests(
  klockfile_testlock
)

########### module for klibloadertest ###############

if(NOT KDE_NO_DEPRECATED)
  set(klibloadertestmodule_PART_SRCS klibloadertest_module.cpp )

  add_library(klibloadertestmodule MODULE ${klibloadertestmodule_PART_SRCS})
  set_target_properties(klibloadertestmodule PROPERTIES PREFIX "") # remove lib prefix - missing from cmake

  target_link_libraries(klibloadertestmodule kde4support kdecore ki18n Qt5::Test)

  set_target_properties(klibloadertestmodule PROPERTIES SKIP_BUILD_RPATH FALSE BUILD_WITH_INSTALL_RPATH FALSE)
endif(NOT KDE_NO_DEPRECATED)

########### module for klibloadertest4 ###############

set(klibloadertestmodule4_PART_SRCS klibloadertest4_module.cpp )

add_library(klibloadertestmodule4 MODULE ${klibloadertestmodule4_PART_SRCS})
set_target_properties(klibloadertestmodule4 PROPERTIES PREFIX "") # remove lib prefix - missing from cmake

target_link_libraries(klibloadertestmodule4 kde4support kdecore ki18n Qt5::Test)

set_target_properties(klibloadertestmodule4 PROPERTIES SKIP_BUILD_RPATH FALSE BUILD_WITH_INSTALL_RPATH FALSE)


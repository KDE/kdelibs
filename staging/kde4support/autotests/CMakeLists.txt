include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/../src/kdecore
   ${CMAKE_CURRENT_SOURCE_DIR}/../src/kdeui
   ${CMAKE_CURRENT_BINARY_DIR}/../src
   ${KDE4_KDECORE_INCLUDES}
   ${KDE4_KDEUI_INCLUDES}
   ${ki18n_SOURCE_DIR}/src
   ${ki18n_BINARY_DIR}/src
)

include(ECMMarkAsTest)

macro(KDE4SUPPORT_UNIT_TESTS)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    add_test(kde4support-${_testname} ${_testname})
    target_link_libraries(${_testname} kde4support ${QT_QTTEST_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTMAIN_LIBRARY} kdecore ${KCoreAddons_LIBRARIES} ki18n kdeui)
    ecm_mark_as_test(${_testname})
  endforeach(_testname)
endmacro(KDE4SUPPORT_UNIT_TESTS)

kde4support_unit_tests(
  kdialog_unittest
  kfadewidgeteffecttest
  kmd5benchmark
  ktempdirtest
  ktemporaryfiletest
  kuniqueapptest
  kurlmimetest
  klibloadertest
  ksavefiletest
  kconfigcompattest
  kmimetypetest
  kmimetype_nomimetypes
  kstandarddirstest
  kasciitest
  kglobaltest
)

########### module for klibloadertest ###############

if(NOT KDE_NO_DEPRECATED)
  set(klibloadertestmodule_PART_SRCS klibloadertest_module.cpp )

  add_library(klibloadertestmodule MODULE ${klibloadertestmodule_PART_SRCS})
  set_target_properties(klibloadertestmodule PROPERTIES PREFIX "") # remove lib prefix - missing from cmake

  target_link_libraries(klibloadertestmodule kde4support kdecore ki18n ${QT_QTTEST_LIBRARY})

  set_target_properties(klibloadertestmodule PROPERTIES SKIP_BUILD_RPATH FALSE BUILD_WITH_INSTALL_RPATH FALSE)
endif(NOT KDE_NO_DEPRECATED)

########### module for klibloadertest4 ###############

set(klibloadertestmodule4_PART_SRCS klibloadertest4_module.cpp )

add_library(klibloadertestmodule4 MODULE ${klibloadertestmodule4_PART_SRCS})
set_target_properties(klibloadertestmodule4 PROPERTIES PREFIX "") # remove lib prefix - missing from cmake

target_link_libraries(klibloadertestmodule4 kde4support kdecore ki18n ${QT_QTTEST_LIBRARY})

set_target_properties(klibloadertestmodule4 PROPERTIES SKIP_BUILD_RPATH FALSE BUILD_WITH_INSTALL_RPATH FALSE)


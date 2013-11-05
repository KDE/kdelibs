
include(ECMMarkAsTest)

include_directories("${CMAKE_CURRENT_BINARY_DIR}/../../..")

macro(KDE4SUPPORT_EXECUTABLE_TESTS)
    foreach(_testname ${ARGN})
        add_executable(${_testname} ${_testname}.cpp)
        target_link_libraries(${_testname} KDE4Support Qt5::Core Qt5::Widgets KF5::KConfigCore)
        ecm_mark_as_test(${_testname})
    endforeach()
endmacro()

macro(KDE4SUPPORT_UNIT_TESTS)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    add_test(kde4support-${_testname} ${_testname})
    target_link_libraries(${_testname}
       KDE4Support
       Qt5::DBus
       Qt5::Widgets
       Qt5::Test
       Qt5::Network
       Qt5::Concurrent
       KF5::KCoreAddons
       KF5::KI18n
       KCrash
       KIOCore
       KIOWidgets
    )
    ecm_mark_as_test(${_testname})
  endforeach(_testname)
endmacro(KDE4SUPPORT_UNIT_TESTS)

kde4support_unit_tests(
  globalcleanuptest
  kaccelgentest
  kcalendartest
  kdatetimetest
  klocaletimeformattest
  ktimezonestest
  k4timezonewidget_unittest
  kdebugtest
  kbuttongrouptest
  kdialog_unittest
  kfadewidgeteffecttest
  kconfigafterkglobaltest1
  kconfigafterkglobaltest2
  kdebug_unittest
  kdebug_qcoreapptest
  kmd5benchmark
  ktabwidget_unittest
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
  kurltest
  kfilemetainfotest
  kglobalsettingstest
  ksharedptrtest
  naturalcomparetest
)

kde4support_executable_tests(
  klockfile_testlock         # Helper for klockfiletest
  kglobalsettingsclient      # Helper for kglobalsettingstest
)

target_include_directories(ktimezonestest PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/../src/kdecore")

########### module for klibloadertest ###############

if(NOT KDE_NO_DEPRECATED)
  set(klibloadertestmodule_PART_SRCS klibloadertest_module.cpp )

  add_library(klibloadertestmodule MODULE ${klibloadertestmodule_PART_SRCS})
  set_target_properties(klibloadertestmodule PROPERTIES PREFIX "") # remove lib prefix - missing from cmake

  target_link_libraries(klibloadertestmodule KDE4Support KF5::KI18n Qt5::Test KF5::KService KF5::KCoreAddons)

  set_target_properties(klibloadertestmodule PROPERTIES SKIP_BUILD_RPATH FALSE BUILD_WITH_INSTALL_RPATH FALSE)
endif(NOT KDE_NO_DEPRECATED)

########### module for klibloadertest4 ###############

set(klibloadertestmodule4_PART_SRCS klibloadertest4_module.cpp )

add_library(klibloadertestmodule4 MODULE ${klibloadertestmodule4_PART_SRCS})
ecm_mark_as_test(klibloadertestmodule4)
set_target_properties(klibloadertestmodule4 PROPERTIES PREFIX "") # remove lib prefix - missing from cmake

target_link_libraries(klibloadertestmodule4 KDE4Support KF5::KI18n Qt5::Test KF5::KService KF5::KCoreAddons)

set_target_properties(klibloadertestmodule4 PROPERTIES SKIP_BUILD_RPATH FALSE BUILD_WITH_INSTALL_RPATH FALSE)

########### klocaletest ###############

# compile into the test since it's not exported
set(klocaletest_SRCS klocaletest.cpp ../src/kdecore/kdayperiod.cpp)
add_executable(klocaletest ${klocaletest_SRCS})
add_test(kde4support-klocaletest klocaletest)
ecm_mark_as_test(klocaletest)
target_link_libraries(klocaletest Qt5::Test Qt5::DBus KF5::KI18n KDE4Support KConfigCore)

########### kdatetimeformattertest ###############

# compile KDateTimeFormatter and KDayPeriod into the test since it's not exported
set(kdatetimeformattertest_SRCS kdatetimeformattertest.cpp ../src/kdecore/kdatetimeformatter.cpp ../src/kdecore/kdayperiod.cpp)
add_executable(kdatetimeformattertest ${kdatetimeformattertest_SRCS})
add_test(kde4support-kdatetimeformattertest kdatetimeformattertest)
ecm_mark_as_test(kdatetimeformattertest)
target_link_libraries(kdatetimeformattertest Qt5::Test KF5::KI18n KDE4Support KConfigCore)



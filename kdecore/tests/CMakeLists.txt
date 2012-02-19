set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${KDE4_KDECORE_INCLUDES} )
remove_definitions(-DQT_NO_CAST_FROM_ASCII)

MACRO(KDECORE_UNIT_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_add_unit_test(${_testname} TESTNAME "kdecore-${_testname}" NOGUI ${_testname}.cpp)
               target_link_libraries(${_testname} ${KDE4_KDECORE_LIBS} ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY} ${QT_QTNETWORK_LIBRARY} kcoreaddons kde4support)
               if(WINCE)
                target_link_libraries(${_testname} ${WCECOMPAT_LIBRARIES})
               endif(WINCE)
       ENDFOREACH(_testname)
ENDMACRO(KDECORE_UNIT_TESTS)
MACRO(KDECORE_EXECUTABLE_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_add_executable(${_testname} NOGUI TEST ${_testname}.cpp)
               target_link_libraries(${_testname} ${KDE4_KDECORE_LIBS} ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY} kcoreaddons kde4support)
               if(WINCE)
                target_link_libraries(${_testname} ${WCECOMPAT_LIBRARIES})
               endif(WINCE)
       ENDFOREACH(_testname)
ENDMACRO(KDECORE_EXECUTABLE_TESTS)


########### next target ###############

KDECORE_UNIT_TESTS(
 kdirwatch_unittest
 klocaletimeformattest
 klocalizedstringtest
 kmountpointtest
 kurltest
 kstringhandlertest
 cplusplustest
 ksortablelisttest
 kmacroexpandertest
 kshelltest
 kasciitest
 ktimezonestest
 kentrymaptest
 ksharedptrtest
 kdatetimetest
 ksavefiletest
 kdesktopfiletest
 kautostarttest
 ksycocadicttest
 kglobalstatictest
 globalcleanuptest
 kprocesstest
 kconfigafterkglobaltest1
 kconfigafterkglobaltest2
 ktcpsockettest
 ksycocathreadtest
 kdebug_unittest
 kencodingdetectortest
 qcoreapptest
 kdebug_qcoreapptest
 kmimetype_nomimetypes
)

if(NOT QT5_BUILD)
  KDECORE_UNIT_TESTS(
    kcalendartest
    kstandarddirstest
    kservicetest
    kconfigtest
    kglobaltest
  )
endif()

if(NOT KDE_NO_DEPRECATED)
 KDECORE_UNIT_TESTS(
  klibloadertest
 )
endif(NOT KDE_NO_DEPRECATED)

if(UNIX)
 KDECORE_UNIT_TESTS(
  klocalsockettest
  klocalsocketservertest
 )
endif(UNIX)

KDECORE_EXECUTABLE_TESTS(
 kdirwatchtest
 kdebugtest
 kmemtest
 dbuscalltest
 kmdcodectest
 startserviceby
)

########### klocaletest ###############

# compile into the test since it's not exported
set(klocaletest_SRCS klocaletest.cpp ../date/kdayperiod.cpp)
kde4_add_unit_test(klocaletest ${klocaletest_SRCS})
target_link_libraries(klocaletest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} kcoreaddons)

########### kdatetimeformattertest ###############

# compile KDateTimeFormatter and KDayPeriod into the test since it's not exported
set(kdatetimeformattertest_SRCS kdatetimeformattertest.cpp ../date/kdatetimeformatter.cpp ../date/kdayperiod.cpp)
kde4_add_unit_test(kdatetimeformattertest ${kdatetimeformattertest_SRCS})
target_link_libraries(kdatetimeformattertest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} kcoreaddons)

########### kdirwatchtest_gui ###############

kde4_add_executable(kdirwatchtest_gui TEST kdirwatchtest_gui.cpp)
target_link_libraries(kdirwatchtest_gui ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY} kcoreaddons)

########### kmimetypetest ###############

if(NOT QT5_BUILD)
  set(kmimetypetest_SRCS kmimetypetest.cpp)
  kde4_add_unit_test(kmimetypetest TESTNAME kdecore-kmimetypetest ${kmimetypetest_SRCS})
  target_link_libraries(kmimetypetest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} kcoreaddons)
endif()

########### module for klibloadertest ###############

if(NOT KDE_NO_DEPRECATED)
set(klibloadertestmodule_PART_SRCS klibloadertest_module.cpp )

kde4_add_plugin(klibloadertestmodule ${klibloadertestmodule_PART_SRCS})

target_link_libraries(klibloadertestmodule  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

set_target_properties(klibloadertestmodule PROPERTIES SKIP_BUILD_RPATH FALSE BUILD_WITH_INSTALL_RPATH FALSE)
endif(NOT KDE_NO_DEPRECATED)

########### module for klibloadertest4 ###############

if (NOT WIN32) # TODO: reenable for win32

set(klibloadertestmodule4_PART_SRCS klibloadertest4_module.cpp )

kde4_add_plugin(klibloadertestmodule4 ${klibloadertestmodule4_PART_SRCS})

target_link_libraries(klibloadertestmodule4  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

set_target_properties(klibloadertestmodule4 PROPERTIES SKIP_BUILD_RPATH FALSE BUILD_WITH_INSTALL_RPATH FALSE)

endif (NOT WIN32)


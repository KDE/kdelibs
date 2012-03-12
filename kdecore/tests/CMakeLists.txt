set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${KDE4_KDECORE_INCLUDES} )
remove_definitions(-DQT_NO_CAST_FROM_ASCII)

MACRO(KDECORE_UNIT_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_add_unit_test(${_testname} TESTNAME "kdecore-${_testname}" NOGUI ${_testname}.cpp)
               target_link_libraries(${_testname} ${KDE4_KDECORE_LIBS} ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY} ${QT_QTNETWORK_LIBRARY} kcoreaddons)
               if(WINCE)
                target_link_libraries(${_testname} ${WCECOMPAT_LIBRARIES})
               endif(WINCE)
       ENDFOREACH(_testname)
ENDMACRO(KDECORE_UNIT_TESTS)
MACRO(KDECORE_EXECUTABLE_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_add_executable(${_testname} NOGUI TEST ${_testname}.cpp)
               target_link_libraries(${_testname} ${KDE4_KDECORE_LIBS} ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY} kcoreaddons)
               if(WINCE)
                target_link_libraries(${_testname} ${WCECOMPAT_LIBRARIES})
               endif(WINCE)
       ENDFOREACH(_testname)
ENDMACRO(KDECORE_EXECUTABLE_TESTS)


########### next target ###############

KDECORE_UNIT_TESTS(
 klocaletimeformattest
 klocalizedstringtest
 kmountpointtest
 kurltest
 cplusplustest
 ksortablelisttest
 kshelltest
 kasciitest
 ktimezonestest
 kentrymaptest
 ksharedptrtest
 kdatetimetest
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

if(UNIX)
 KDECORE_UNIT_TESTS(
  klocalsockettest
  klocalsocketservertest
 )
endif(UNIX)

KDECORE_EXECUTABLE_TESTS(
 kdebugtest
 kmemtest
 dbuscalltest
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

########### kmimetypetest ###############

if(NOT QT5_BUILD)
  set(kmimetypetest_SRCS kmimetypetest.cpp)
  kde4_add_unit_test(kmimetypetest TESTNAME kdecore-kmimetypetest ${kmimetypetest_SRCS})
  target_link_libraries(kmimetypetest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} kcoreaddons)
endif()


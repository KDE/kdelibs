set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${KDE4_KDECORE_INCLUDES} )
remove_definitions(-DQT_NO_CAST_FROM_ASCII)

MACRO(KDECORE_UNIT_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_add_unit_test(${_testname} TESTNAME "kdecore-${_testname}" NOGUI ${_testname}.cpp)
               target_link_libraries(${_testname} ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} ${QT_QTNETWORK_LIBRARY})
               if(WINCE)
                target_link_libraries(${_testname} ${WCECOMPAT_LIBRARIES})
               endif(WINCE)
       ENDFOREACH(_testname)
ENDMACRO(KDECORE_UNIT_TESTS)
MACRO(KDECORE_EXECUTABLE_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_add_executable(${_testname} NOGUI TEST ${_testname}.cpp)
               target_link_libraries(${_testname} ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})
               if(WINCE)
                target_link_libraries(${_testname} ${WCECOMPAT_LIBRARIES})
               endif(WINCE)
       ENDFOREACH(_testname)
ENDMACRO(KDECORE_EXECUTABLE_TESTS)


########### next target ###############

KDECORE_UNIT_TESTS(
 karchivetest
 kdirwatch_unittest
 klocaletimeformattest
 klocalizedstringtest
 kmountpointtest
 kstandarddirstest
 kaboutdatatest
 kurltest
 kstringhandlertest
 cplusplustest
 ksortablelisttest
 kcharsetstest
 kcalendartest
 kmacroexpandertest
 kshelltest
 kasciitest
 ktimezonestest
 kentrymaptest
 kconfigtest
 kurlmimetest
 klockfiletest
 ktempdirtest
 ksharedptrtest
 kshareddatacachetest
 kdatetimetest
 ksavefiletest
 kautosavefiletest
 kdesktopfiletest
 ktemporaryfiletest
 kautostarttest
 kjobtest
 ksycocadicttest
 kservicetest
 kglobalstatictest
 kglobaltest
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
 kmd5benchmark
)

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
 krandomsequencetest
 ktartest
 kziptest
 kdebugtest
 kcmdlineargstest
 kmemtest
 dbuscalltest
 kmdcodectest
 startserviceby
 klockfile_testlock # helper for klockfiletest
)

########### klocaletest ###############

# compile into the test since it's not exported
set(klocaletest_SRCS klocaletest.cpp ../date/kdayperiod.cpp)
kde4_add_unit_test(klocaletest ${klocaletest_SRCS})
target_link_libraries(klocaletest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} )

########### kdatetimeformattertest ###############

# compile KDateTimeFormatter and KDayPeriod into the test since it's not exported
set(kdatetimeformattertest_SRCS kdatetimeformattertest.cpp ../date/kdatetimeformatter.cpp ../date/kdayperiod.cpp)
kde4_add_unit_test(kdatetimeformattertest ${kdatetimeformattertest_SRCS})
target_link_libraries(kdatetimeformattertest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} )

########### kdirwatchtest_gui ###############

kde4_add_executable(kdirwatchtest_gui TEST kdirwatchtest_gui.cpp)
target_link_libraries(kdirwatchtest_gui ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY})

########### klimitediodevicetest ###############

kde4_add_unit_test(klimitediodevicetest TESTNAME kdecore-klimitediodevicetest klimitediodevicetest.cpp ../io/klimitediodevice.cpp)
target_link_libraries(klimitediodevicetest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

########### kmimetypetest ###############

# compile kmimemagicrule.cpp into the test since it's not exported and we call match().
set(kmimetypetest_SRCS kmimetypetest.cpp ../services/kmimemagicrule.cpp)
kde4_add_unit_test(kmimetypetest TESTNAME kdecore-kmimetypetest ${kmimetypetest_SRCS})
target_link_libraries(kmimetypetest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} )

########### kmimeglobsfileparsertest ###############

# compile kmimeglobsfileparser.cpp into the test since it's not exported
set(kmimeglobsfileparsertest_SRCS kmimeglobsfileparsertest.cpp ../services/kmimeglobsfileparser.cpp)
kde4_add_unit_test(kmimeglobsfileparsertest TESTNAME kdecore-kmimeglobsfileparsertest ${kmimeglobsfileparsertest_SRCS})
target_link_libraries(kmimeglobsfileparsertest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} )

########### kfiltertest ###############

# compile httpfilter.cpp into the test since it's not part of kdelibs
# (only par of kio_http and kmultipart)
set(kfiltertest_SRCS kfiltertest.cpp ../../kio/httpfilter/httpfilter.cc)
include_directories( ${CMAKE_SOURCE_DIR}/kio/httpfilter )
kde4_add_unit_test(kfiltertest TESTNAME kdecore-kfiltertest ${kfiltertest_SRCS})
target_link_libraries(kfiltertest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} )
target_link_libraries(kfiltertest ${ZLIB_LIBRARIES})

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

########### kcompositejobtest ###############

set(kcompositejobtest_SRCS kcompositejobtest.cpp)
kde4_add_unit_test(kcompositejobtest TESTNAME kdecore-kcompositejobtest ${kcompositejobtest_SRCS})
target_link_libraries(kcompositejobtest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} )

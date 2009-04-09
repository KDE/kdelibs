set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${KDE4_KDECORE_INCLUDES} )

MACRO(KDECORE_UNIT_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_add_unit_test(${_testname} NOGUI ${_testname}.cpp)
               target_link_libraries(${_testname} ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} ${QT_QTNETWORK_LIBRARY})
       ENDFOREACH(_testname)
ENDMACRO(KDECORE_UNIT_TESTS)
MACRO(KDECORE_EXECUTABLE_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_add_executable(${_testname} NOGUI TEST ${_testname}.cpp)
               target_link_libraries(${_testname} ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})
       ENDFOREACH(_testname)
ENDMACRO(KDECORE_EXECUTABLE_TESTS)


########### next target ###############

KDECORE_UNIT_TESTS(
 klocaletest
 klocalizedstringtest
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
 klibloadertest
 klockfiletest
 ktempdirtest
 ksharedptrtest
 kfiltertest
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
)

target_link_libraries(kfiltertest ${ZLIB_LIBRARIES})

if(UNIX)
 KDECORE_UNIT_TESTS(
  klocalsockettest
  klocalsocketservertest
 )
endif(UNIX)

KDECORE_EXECUTABLE_TESTS(
 krandomsequencetest
 kdebugtest
 kcmdlineargstest
 kmemtest
 dbuscalltest
 kmdcodectest
 startserviceby
)

########### kmimetypetest ###############

# compile kmimemagicrule.cpp into the test since it's not exported and we call match().
set(kmimetypetest_SRCS kmimetypetest.cpp ../services/kmimemagicrule.cpp)
kde4_add_unit_test(kmimetypetest ${kmimetypetest_SRCS})
target_link_libraries(kmimetypetest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} )

########### module for klibloadertest ###############

set(klibloadertestmodule_PART_SRCS klibloadertest_module.cpp )

kde4_add_plugin(klibloadertestmodule ${klibloadertestmodule_PART_SRCS})

target_link_libraries(klibloadertestmodule  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

set_target_properties(klibloadertestmodule PROPERTIES SKIP_BUILD_RPATH FALSE BUILD_WITH_INSTALL_RPATH FALSE)

########### module for klibloadertest4 ###############

if (NOT WIN32) # TODO: reenable for win32

set(klibloadertestmodule4_PART_SRCS klibloadertest4_module.cpp )

kde4_add_plugin(klibloadertestmodule4 ${klibloadertestmodule4_PART_SRCS})

target_link_libraries(klibloadertestmodule4  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

set_target_properties(klibloadertestmodule4 PROPERTIES SKIP_BUILD_RPATH FALSE BUILD_WITH_INSTALL_RPATH FALSE)

endif (NOT WIN32)

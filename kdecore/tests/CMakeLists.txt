kde4_header()

if(KDE4_BUILD_TESTS)

include_directories( ${CMAKE_SOURCE_DIR}/kdecore ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kconfigtestgui_SRCS kconfigtestgui.cpp )

kde4_automoc(${kconfigtestgui_SRCS})

kde4_add_executable(kconfigtestgui ${kconfigtestgui_SRCS})

target_link_libraries(kconfigtestgui  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-kconfigtestgui ${EXECUTABLE_OUTPUT_PATH}/kconfigtestgui)

########### next target ###############

set(klocaletest_SRCS klocaletest.cpp )

kde4_automoc(${klocaletest_SRCS})

kde4_add_executable(klocaletest ${klocaletest_SRCS})

target_link_libraries(klocaletest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-klocaletest ${EXECUTABLE_OUTPUT_PATH}/klocaletest)

########### next target ###############

set(kprocesstest_SRCS kprocesstest.cpp )

kde4_automoc(${kprocesstest_SRCS})

kde4_add_executable(kprocesstest ${kprocesstest_SRCS})

target_link_libraries(kprocesstest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-kprocesstest ${EXECUTABLE_OUTPUT_PATH}/kprocesstest)

########### next target ###############

set(ksimpleconfigtest_SRCS ksimpleconfigtest.cpp )

kde4_automoc(${ksimpleconfigtest_SRCS})

kde4_add_executable(ksimpleconfigtest ${ksimpleconfigtest_SRCS})

target_link_libraries(ksimpleconfigtest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-ksimpleconfigtest ${EXECUTABLE_OUTPUT_PATH}/ksimpleconfigtest)

########### next target ###############

set(kstddirstest_SRCS kstddirstest.cpp )

kde4_automoc(${kstddirstest_SRCS})

kde4_add_executable(kstddirstest ${kstddirstest_SRCS})

target_link_libraries(kstddirstest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-kstddirstest ${EXECUTABLE_OUTPUT_PATH}/kstddirstest)

########### next target ###############

#set(kurltest_SRCS kurltest.cpp )
#
#kde4_automoc(${kurltest_SRCS})
#
#kde4_add_executable(kurltest ${kurltest_SRCS})
#
#target_link_libraries(kurltest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})
#
# add_test(kdecore-kurltest ${EXECUTABLE_OUTPUT_PATH}/kurltest)

########### next target ###############

set(kurltest4_SRCS kurltest4.cpp )

kde4_automoc(${kurltest4_SRCS})

kde4_add_executable(kurltest4 ${kurltest4_SRCS})

target_link_libraries(kurltest4  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-kurltest4 ${EXECUTABLE_OUTPUT_PATH}/kurltest4)

########### next target ###############

set(kuniqueapptest_SRCS kuniqueapptest.cpp )

kde4_automoc(${kuniqueapptest_SRCS})

kde4_add_executable(kuniqueapptest ${kuniqueapptest_SRCS})

target_link_libraries(kuniqueapptest  ${KDE4_KDECORE_LIBS} )

# add_test(kdecore-kuniqueapptest ${EXECUTABLE_OUTPUT_PATH}/kuniqueapptest)

########### next target ###############

set(krandomsequencetest_SRCS krandomsequencetest.cpp )

kde4_automoc(${krandomsequencetest_SRCS})

kde4_add_executable(krandomsequencetest ${krandomsequencetest_SRCS})

target_link_libraries(krandomsequencetest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-krandomsequencetest ${EXECUTABLE_OUTPUT_PATH}/krandomsequencetest)

########### next target ###############

set(kdebugtest_SRCS kdebugtest.cpp )

kde4_automoc(${kdebugtest_SRCS})

kde4_add_executable(kdebugtest ${kdebugtest_SRCS})

target_link_libraries(kdebugtest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-kdebugtest ${EXECUTABLE_OUTPUT_PATH}/kdebugtest)

########### next target ###############

set(kstringhandlertest_SRCS kstringhandlertest.cpp )

kde4_automoc(${kstringhandlertest_SRCS})

kde4_add_executable(kstringhandlertest ${kstringhandlertest_SRCS})

target_link_libraries(kstringhandlertest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-kstringhandlertest ${EXECUTABLE_OUTPUT_PATH}/kstringhandlertest)

########### next target ###############

set(kcmdlineargstest_SRCS kcmdlineargstest.cpp )

kde4_automoc(${kcmdlineargstest_SRCS})

kde4_add_executable(kcmdlineargstest ${kcmdlineargstest_SRCS})

target_link_libraries(kcmdlineargstest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-kcmdlineargstest ${EXECUTABLE_OUTPUT_PATH}/kcmdlineargstest)

########### next target ###############

set(kapptest_SRCS kapptest.cpp )

kde4_automoc(${kapptest_SRCS})

kde4_add_executable(kapptest ${kapptest_SRCS})

target_link_libraries(kapptest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-kapptest ${EXECUTABLE_OUTPUT_PATH}/kapptest)

########### next target ###############

set(kmemtest_SRCS kmemtest.cpp )

kde4_automoc(${kmemtest_SRCS})

kde4_add_executable(kmemtest ${kmemtest_SRCS})

target_link_libraries(kmemtest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-kmemtest ${EXECUTABLE_OUTPUT_PATH}/kmemtest)

########### next target ###############

set(kidlservertest_SRCS KIDLTest.cpp )

kde4_automoc(${kidlservertest_SRCS})

set( kidlservertest_DCOP_SKEL_SRCS
KIDLTest.h
)

kde4_add_dcop_skels(kidlservertest_SRCS ${kidlservertest_DCOP_SKEL_SRCS})

kde4_add_executable(kidlservertest ${kidlservertest_SRCS})

target_link_libraries(kidlservertest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-kidlservertest ${EXECUTABLE_OUTPUT_PATH}/kidlservertest)

########### next target ###############

set(kidlclienttest_SRCS KIDLTestClient.cpp )

kde4_automoc(${kidlclienttest_SRCS})

set( kidlclienttest_DCOP_STUB_SRCS KIDLTest.h )

kde4_add_dcop_stubs(kidlclienttest_SRCS ${kidlclienttest_DCOP_STUB_SRCS})

kde4_add_executable(kidlclienttest ${kidlclienttest_SRCS})

target_link_libraries(kidlclienttest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-kidlclienttest ${EXECUTABLE_OUTPUT_PATH}/kidlclienttest)

########### next target ###############

set(dcopkonqtest_SRCS dcopkonqtest.cpp )

kde4_automoc(${dcopkonqtest_SRCS})

kde4_add_executable(dcopkonqtest ${dcopkonqtest_SRCS})

target_link_libraries(dcopkonqtest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-dcopkonqtest ${EXECUTABLE_OUTPUT_PATH}/dcopkonqtest)

########### next target ###############

set(kipctest_SRCS kipctest.cpp )

kde4_automoc(${kipctest_SRCS})

kde4_add_executable(kipctest ${kipctest_SRCS})

target_link_libraries(kipctest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-kipctest ${EXECUTABLE_OUTPUT_PATH}/kipctest)

########### next target ###############

set(cplusplustest_SRCS cplusplustest.cpp )

kde4_automoc(${cplusplustest_SRCS})

kde4_add_executable(cplusplustest ${cplusplustest_SRCS})

target_link_libraries(cplusplustest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-cplusplustest ${EXECUTABLE_OUTPUT_PATH}/cplusplustest)

########### next target ###############

set(kiconloadertest_SRCS kiconloadertest.cpp )

kde4_automoc(${kiconloadertest_SRCS})

kde4_add_executable(kiconloadertest ${kiconloadertest_SRCS})

target_link_libraries(kiconloadertest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-kiconloadertest ${EXECUTABLE_OUTPUT_PATH}/kiconloadertest)

########### next target ###############

set(kmdcodectest_SRCS kmdcodectest.cpp )

kde4_automoc(${kmdcodectest_SRCS})

kde4_add_executable(kmdcodectest ${kmdcodectest_SRCS})

target_link_libraries(kmdcodectest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-kmdcodectest ${EXECUTABLE_OUTPUT_PATH}/kmdcodectest)

########### next target ###############

set(knotifytest_SRCS knotifytest.cpp )

kde4_automoc(${knotifytest_SRCS})

kde4_add_executable(knotifytest ${knotifytest_SRCS})

target_link_libraries(knotifytest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-knotifytest ${EXECUTABLE_OUTPUT_PATH}/knotifytest)

########### next target ###############

set(kdatetimetest_SRCS kdatetimetest.cpp )

kde4_automoc(${kdatetimetest_SRCS})

kde4_add_executable(kdatetimetest ${kdatetimetest_SRCS})

target_link_libraries(kdatetimetest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-kdatetimetest ${EXECUTABLE_OUTPUT_PATH}/kdatetimetest)

########### next target ###############

set(ksortablevaluelisttest_SRCS ksortablevaluelisttest.cpp )

kde4_automoc(${ksortablevaluelisttest_SRCS})

kde4_add_executable(ksortablevaluelisttest ${ksortablevaluelisttest_SRCS})

target_link_libraries(ksortablevaluelisttest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-ksortablevaluelisttest ${EXECUTABLE_OUTPUT_PATH}/ksortablevaluelisttest)

########### next target ###############

set(krfcdatetest_SRCS krfcdatetest.cpp )

kde4_automoc(${krfcdatetest_SRCS})

kde4_add_executable(krfcdatetest ${krfcdatetest_SRCS})

target_link_libraries(krfcdatetest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-krfcdatetest ${EXECUTABLE_OUTPUT_PATH}/krfcdatetest)

########### next target ###############

set(testqtargs_SRCS testqtargs.cpp )

kde4_automoc(${testqtargs_SRCS})

kde4_add_executable(testqtargs ${testqtargs_SRCS})

target_link_libraries(testqtargs  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-testqtargs ${EXECUTABLE_OUTPUT_PATH}/testqtargs)

########### next target ###############

set(kprociotest_SRCS kprociotest.cpp )

kde4_automoc(${kprociotest_SRCS})

kde4_add_executable(kprociotest ${kprociotest_SRCS})

target_link_libraries(kprociotest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-kprociotest ${EXECUTABLE_OUTPUT_PATH}/kprociotest)

########### next target ###############

set(kcharsetstest_SRCS kcharsetstest.cpp )

kde4_automoc(${kcharsetstest_SRCS})

kde4_add_executable(kcharsetstest ${kcharsetstest_SRCS})

target_link_libraries(kcharsetstest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-kcharsetstest ${EXECUTABLE_OUTPUT_PATH}/kcharsetstest)

########### next target ###############

set(kcalendartest_SRCS kcalendartest.cpp )

kde4_automoc(${kcalendartest_SRCS})

kde4_add_executable(kcalendartest ${kcalendartest_SRCS})

target_link_libraries(kcalendartest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-kcalendartest ${EXECUTABLE_OUTPUT_PATH}/kcalendartest)

########### next target ###############

set(kmacroexpandertest_SRCS kmacroexpandertest.cpp )

kde4_automoc(${kmacroexpandertest_SRCS})

kde4_add_executable(kmacroexpandertest ${kmacroexpandertest_SRCS})

target_link_libraries(kmacroexpandertest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-kmacroexpandertest ${EXECUTABLE_OUTPUT_PATH}/kmacroexpandertest)

########### next target ###############

set(kshelltest_SRCS kshelltest.cpp )

kde4_automoc(${kshelltest_SRCS})

kde4_add_executable(kshelltest ${kshelltest_SRCS})

target_link_libraries(kshelltest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-kshelltest ${EXECUTABLE_OUTPUT_PATH}/kshelltest)

########### next target ###############

if (UNIX)
set(kxerrorhandlertest_SRCS kxerrorhandlertest.cpp )

kde4_automoc(${kxerrorhandlertest_SRCS})

kde4_add_executable(kxerrorhandlertest ${kxerrorhandlertest_SRCS})

target_link_libraries(kxerrorhandlertest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-kxerrorhandlertest ${EXECUTABLE_OUTPUT_PATH}/kxerrorhandlertest)
endif (UNIX)

########### next target ###############

set(startserviceby_SRCS startserviceby.cpp )

kde4_automoc(${startserviceby_SRCS})

kde4_add_executable(startserviceby ${startserviceby_SRCS})

target_link_libraries(startserviceby  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-startserviceby ${EXECUTABLE_OUTPUT_PATH}/startserviceby)

########### next target ###############

set(kglobaltest_SRCS kglobaltest.cpp )

kde4_automoc(${kglobaltest_SRCS})

kde4_add_executable(kglobaltest ${kglobaltest_SRCS})

target_link_libraries(kglobaltest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-kglobaltest ${EXECUTABLE_OUTPUT_PATH}/kglobaltest)

########### next target ###############

set(ktimezonestest_SRCS ktimezonestest.cpp )

kde4_automoc(${ktimezonestest_SRCS})

kde4_add_executable(ktimezonestest ${ktimezonestest_SRCS})

target_link_libraries(ktimezonestest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-ktimezonestest ${EXECUTABLE_OUTPUT_PATH}/ktimezonestest)

########### next target ###############

set(kconfigtest_SRCS kconfigtest.cpp )

kde4_automoc(${kconfigtest_SRCS})

kde4_add_executable(kconfigtest ${kconfigtest_SRCS})

target_link_libraries(kconfigtest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-kconfigtest ${EXECUTABLE_OUTPUT_PATH}/kconfigtest)

########### next target ###############

set(ktempfiletest_SRCS ktempfiletest.cpp )

kde4_automoc(${ktempfiletest_SRCS})

kde4_add_executable(ktempfiletest ${ktempfiletest_SRCS})

target_link_libraries(ktempfiletest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-ktempfiletest ${EXECUTABLE_OUTPUT_PATH}/ktempfiletest)

########### next target ###############

# set(kurlmimetest_SRCS kurlmimetest.cpp )
#
# kde4_automoc(${kurlmimetest_SRCS})
#
# kde4_add_executable(kurlmimetest ${kurlmimetest_SRCS})
#
# target_link_libraries(kurlmimetest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})
#
# add_test(kdecore-kurlmimetest ${EXECUTABLE_OUTPUT_PATH}/kurlmimetest)


########### next target ###############

set(kstdacceltest_SRCS kstdacceltest.cpp )

kde4_automoc(${kstdacceltest_SRCS})

kde4_add_executable(kstdacceltest ${kstdacceltest_SRCS})

target_link_libraries(kstdacceltest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-kstdacceltest ${EXECUTABLE_OUTPUT_PATH}/kstdacceltest)

########### next target ###############

set(klibloadertest_SRCS klibloadertest.cpp )

kde4_automoc(${klibloadertest_SRCS})

kde4_add_executable(klibloadertest ${klibloadertest_SRCS})

target_link_libraries(klibloadertest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-klibloadertest ${EXECUTABLE_OUTPUT_PATH}/klibloadertest)

########### next target ###############

set(kcompletiontest_SRCS kcompletiontest.cpp )

kde4_automoc(${kcompletiontest_SRCS})

kde4_add_executable(kcompletiontest ${kcompletiontest_SRCS})

target_link_libraries(kcompletiontest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-kcompletiontest ${EXECUTABLE_OUTPUT_PATH}/kcompletiontest)

########### next target ###############

set(klockfiletest_SRCS klockfiletest.cpp )

kde4_automoc(${klockfiletest_SRCS})

kde4_add_executable(lt-klockfiletest ${klockfiletest_SRCS})

target_link_libraries(lt-klockfiletest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-klockfiletest ${EXECUTABLE_OUTPUT_PATH}/lt-klockfiletest)

########### next target ###############

set(ktempdirtest_SRCS ktempdirtest.cpp )

kde4_automoc(${ktempdirtest_SRCS})

kde4_add_executable(ktempdirtest ${ktempdirtest_SRCS})

target_link_libraries(ktempdirtest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-ktempdirtest ${EXECUTABLE_OUTPUT_PATH}/ktempdirtest)

########### next target ###############

set(ksharedptrtest_SRCS ksharedptrtest.cpp )

kde4_automoc(${ksharedptrtest_SRCS})

kde4_add_executable(ksharedptrtest ${ksharedptrtest_SRCS})

target_link_libraries(ksharedptrtest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-ksharedptrtest ${EXECUTABLE_OUTPUT_PATH}/ksharedptrtest)

########### next target ###############

set(kfiltertest_SRCS kfiltertest.cpp )

kde4_automoc(${kfiltertest_SRCS})

kde4_add_executable(kfiltertest ${kfiltertest_SRCS})

target_link_libraries(kfiltertest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-kfiltertest ${EXECUTABLE_OUTPUT_PATH}/kfiltertest)

########### next target ###############

set(kdatetimetest_SRCS dummy.cpp )

kde4_automoc(${kdatetimetest_SRCS})

kde4_add_executable(kdatetimetest ${kdatetimetest_SRCS})

target_link_libraries(kdatetimetest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-kdatetimetest ${EXECUTABLE_OUTPUT_PATH}/kdatetimetest)

########### next target ###############

set(ksavefiletest_SRCS ksavefiletest.cpp )

kde4_automoc(${ksavefiletest_SRCS})

kde4_add_executable(ksavefiletest ${ksavefiletest_SRCS})

target_link_libraries(ksavefiletest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-ksavefiletest ${EXECUTABLE_OUTPUT_PATH}/ksavefiletest)

########### next target ###############

set(klibloadertestmodule_PART_SRCS klibloadertest_module.cpp )

kde4_automoc(${klibloadertestmodule_PART_SRCS})

kde4_add_plugin(klibloadertestmodule WITH_PREFIX ${klibloadertestmodule_PART_SRCS})

target_link_libraries(klibloadertestmodule  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} klibloadertestmodule )
install_targets(${PLUGIN_INSTALL_DIR} klibloadertestmodule )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(knotificationrestrictionstest_SRCS knotificationrestrictionstest.cpp )

kde4_automoc(${knotificationrestrictionstest_SRCS})

kde4_add_executable(knotificationrestrictionstest ${knotificationrestrictionstest_SRCS})

target_link_libraries(knotificationrestrictionstest  ${QT_AND_KDECORE_LIBS})

add_test(kdecore-knotificationrestrictionstest ${EXECUTABLE_OUTPUT_PATH}/knotificationrestrictionstest)

########### next target ###############

set(kautostarttest_SRCS kautostarttest.cpp )


kde4_automoc(${kautostarttest_SRCS})

kde4_add_executable(kautostarttest ${kautostarttest_SRCS})

target_link_libraries(kautostarttest  ${QT_AND_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-kautostarttest ${EXECUTABLE_OUTPUT_PATH}/kautostarttest)

########### install files ###############


kde4_footer()



kde4_header()

if(KDE4_BUILD_TESTS)

include_directories( ${CMAKE_SOURCE_DIR}/kdecore ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kconfigtestgui_SRCS kconfigtestgui.cpp )

kde4_automoc(${kconfigtestgui_SRCS})

kde4_add_executable(kconfigtestgui ${kconfigtestgui_SRCS})

target_link_libraries(kconfigtestgui  ${QT_AND_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-kconfigtestgui ${EXECUTABLE_OUTPUT_PATH}/kconfigtestgui)

########### next target ###############

set(klocaletest_SRCS klocaletest.cpp )

kde4_automoc(${klocaletest_SRCS})

kde4_add_executable(klocaletest ${klocaletest_SRCS})

target_link_libraries(klocaletest  ${QT_AND_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-klocaletest ${EXECUTABLE_OUTPUT_PATH}/klocaletest)

########### next target ###############

set(kprocesstest_SRCS kprocesstest.cpp )

kde4_automoc(${kprocesstest_SRCS})

kde4_add_executable(kprocesstest ${kprocesstest_SRCS})

target_link_libraries(kprocesstest  ${QT_AND_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-kprocesstest ${EXECUTABLE_OUTPUT_PATH}/kprocesstest)

########### next target ###############

set(ksimpleconfigtest_SRCS ksimpleconfigtest.cpp )

kde4_automoc(${ksimpleconfigtest_SRCS})

kde4_add_executable(ksimpleconfigtest ${ksimpleconfigtest_SRCS})

target_link_libraries(ksimpleconfigtest  ${QT_AND_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-ksimpleconfigtest ${EXECUTABLE_OUTPUT_PATH}/ksimpleconfigtest)

########### next target ###############

set(kstddirstest_SRCS kstddirstest.cpp )

kde4_automoc(${kstddirstest_SRCS})

kde4_add_executable(kstddirstest ${kstddirstest_SRCS})

target_link_libraries(kstddirstest  ${QT_AND_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-kstddirstest ${EXECUTABLE_OUTPUT_PATH}/kstddirstest)

########### next target ###############

#set(kurltest_SRCS kurltest.cpp )
#
#kde4_automoc(${kurltest_SRCS})
#
#kde4_add_executable(kurltest ${kurltest_SRCS})
#
#target_link_libraries(kurltest  ${QT_AND_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})
#
# add_test(kdecore-kurltest ${EXECUTABLE_OUTPUT_PATH}/kurltest)

########### next target ###############

set(kurltest4_SRCS kurltest4.cpp )

kde4_automoc(${kurltest4_SRCS})

kde4_add_executable(kurltest4 ${kurltest4_SRCS})

target_link_libraries(kurltest4  ${QT_AND_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-kurltest4 ${EXECUTABLE_OUTPUT_PATH}/kurltest4)

########### next target ###############

set(kuniqueapptest_SRCS kuniqueapptest.cpp )

kde4_automoc(${kuniqueapptest_SRCS})

kde4_add_executable(kuniqueapptest ${kuniqueapptest_SRCS})

target_link_libraries(kuniqueapptest  ${QT_AND_KDECORE_LIBS} )

# add_test(kdecore-kuniqueapptest ${EXECUTABLE_OUTPUT_PATH}/kuniqueapptest)

########### next target ###############

set(krandomsequencetest_SRCS krandomsequencetest.cpp )

kde4_automoc(${krandomsequencetest_SRCS})

kde4_add_executable(krandomsequencetest ${krandomsequencetest_SRCS})

target_link_libraries(krandomsequencetest  ${QT_AND_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-krandomsequencetest ${EXECUTABLE_OUTPUT_PATH}/krandomsequencetest)

########### next target ###############

set(kdebugtest_SRCS kdebugtest.cpp )

kde4_automoc(${kdebugtest_SRCS})

kde4_add_executable(kdebugtest ${kdebugtest_SRCS})

target_link_libraries(kdebugtest  ${QT_AND_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-kdebugtest ${EXECUTABLE_OUTPUT_PATH}/kdebugtest)

########### next target ###############

set(kstringhandlertest_SRCS kstringhandlertest.cpp )

kde4_automoc(${kstringhandlertest_SRCS})

kde4_add_executable(kstringhandlertest ${kstringhandlertest_SRCS})

target_link_libraries(kstringhandlertest  ${QT_AND_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-kstringhandlertest ${EXECUTABLE_OUTPUT_PATH}/kstringhandlertest)

########### next target ###############

set(kcmdlineargstest_SRCS kcmdlineargstest.cpp )

kde4_automoc(${kcmdlineargstest_SRCS})

kde4_add_executable(kcmdlineargstest ${kcmdlineargstest_SRCS})

target_link_libraries(kcmdlineargstest  ${QT_AND_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-kcmdlineargstest ${EXECUTABLE_OUTPUT_PATH}/kcmdlineargstest)

########### next target ###############

set(kapptest_SRCS kapptest.cpp )

kde4_automoc(${kapptest_SRCS})

kde4_add_executable(kapptest ${kapptest_SRCS})

target_link_libraries(kapptest  ${QT_AND_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-kapptest ${EXECUTABLE_OUTPUT_PATH}/kapptest)

########### next target ###############

set(kmemtest_SRCS kmemtest.cpp )

kde4_automoc(${kmemtest_SRCS})

kde4_add_executable(kmemtest ${kmemtest_SRCS})

target_link_libraries(kmemtest  ${QT_AND_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-kmemtest ${EXECUTABLE_OUTPUT_PATH}/kmemtest)

########### next target ###############

set(kidlservertest_SRCS KIDLTest.cpp )

kde4_automoc(${kidlservertest_SRCS})

set( kidlservertest_DCOP_SKEL_SRCS
KIDLTest.h
)

kde4_add_dcop_skels(kidlservertest_SRCS ${kidlservertest_DCOP_SKEL_SRCS})

kde4_add_executable(kidlservertest ${kidlservertest_SRCS})

target_link_libraries(kidlservertest  ${QT_AND_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-kidlservertest ${EXECUTABLE_OUTPUT_PATH}/kidlservertest)

########### next target ###############

set(kidlclienttest_SRCS KIDLTestClient.cpp )

kde4_automoc(${kidlclienttest_SRCS})

set( kidlclienttest_DCOP_STUB_SRCS KIDLTest.h )

kde4_add_dcop_stubs(kidlclienttest_SRCS ${kidlclienttest_DCOP_STUB_SRCS})

kde4_add_executable(kidlclienttest ${kidlclienttest_SRCS})

target_link_libraries(kidlclienttest  ${QT_AND_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-kidlclienttest ${EXECUTABLE_OUTPUT_PATH}/kidlclienttest)

########### next target ###############

set(dcopkonqtest_SRCS dcopkonqtest.cpp )

kde4_automoc(${dcopkonqtest_SRCS})

kde4_add_executable(dcopkonqtest ${dcopkonqtest_SRCS})

target_link_libraries(dcopkonqtest  ${QT_AND_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-dcopkonqtest ${EXECUTABLE_OUTPUT_PATH}/dcopkonqtest)

########### next target ###############

set(kipctest_SRCS kipctest.cpp )

kde4_automoc(${kipctest_SRCS})

kde4_add_executable(kipctest ${kipctest_SRCS})

target_link_libraries(kipctest  ${QT_AND_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-kipctest ${EXECUTABLE_OUTPUT_PATH}/kipctest)

########### next target ###############

set(cplusplustest_SRCS cplusplustest.cpp )

kde4_automoc(${cplusplustest_SRCS})

kde4_add_executable(cplusplustest ${cplusplustest_SRCS})

target_link_libraries(cplusplustest  ${QT_AND_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-cplusplustest ${EXECUTABLE_OUTPUT_PATH}/cplusplustest)

########### next target ###############

set(kiconloadertest_SRCS kiconloadertest.cpp )

kde4_automoc(${kiconloadertest_SRCS})

kde4_add_executable(kiconloadertest ${kiconloadertest_SRCS})

target_link_libraries(kiconloadertest  ${QT_AND_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-kiconloadertest ${EXECUTABLE_OUTPUT_PATH}/kiconloadertest)

########### next target ###############

set(kmdcodectest_SRCS kmdcodectest.cpp )

kde4_automoc(${kmdcodectest_SRCS})

kde4_add_executable(kmdcodectest ${kmdcodectest_SRCS})

target_link_libraries(kmdcodectest  ${QT_AND_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-kmdcodectest ${EXECUTABLE_OUTPUT_PATH}/kmdcodectest)

########### next target ###############

set(knotifytest_SRCS knotifytest.cpp )

kde4_automoc(${knotifytest_SRCS})

kde4_add_executable(knotifytest ${knotifytest_SRCS})

target_link_libraries(knotifytest  ${QT_AND_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-knotifytest ${EXECUTABLE_OUTPUT_PATH}/knotifytest)

########### next target ###############

set(kdatetimetest_SRCS kdatetimetest.cpp )

kde4_automoc(${kdatetimetest_SRCS})

kde4_add_executable(kdatetimetest ${kdatetimetest_SRCS})

target_link_libraries(kdatetimetest  ${QT_AND_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-kdatetimetest ${EXECUTABLE_OUTPUT_PATH}/kdatetimetest)

########### next target ###############

set(ksortablevaluelisttest_SRCS ksortablevaluelisttest.cpp )

kde4_automoc(${ksortablevaluelisttest_SRCS})

kde4_add_executable(ksortablevaluelisttest ${ksortablevaluelisttest_SRCS})

target_link_libraries(ksortablevaluelisttest  ${QT_AND_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-ksortablevaluelisttest ${EXECUTABLE_OUTPUT_PATH}/ksortablevaluelisttest)

########### next target ###############

set(krfcdatetest_SRCS krfcdatetest.cpp )

kde4_automoc(${krfcdatetest_SRCS})

kde4_add_executable(krfcdatetest ${krfcdatetest_SRCS})

target_link_libraries(krfcdatetest  ${QT_AND_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-krfcdatetest ${EXECUTABLE_OUTPUT_PATH}/krfcdatetest)

########### next target ###############

set(testqtargs_SRCS testqtargs.cpp )

kde4_automoc(${testqtargs_SRCS})

kde4_add_executable(testqtargs ${testqtargs_SRCS})

target_link_libraries(testqtargs  ${QT_AND_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-testqtargs ${EXECUTABLE_OUTPUT_PATH}/testqtargs)

########### next target ###############

set(kprociotest_SRCS kprociotest.cpp )

kde4_automoc(${kprociotest_SRCS})

kde4_add_executable(kprociotest ${kprociotest_SRCS})

target_link_libraries(kprociotest  ${QT_AND_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-kprociotest ${EXECUTABLE_OUTPUT_PATH}/kprociotest)

########### next target ###############

set(kcharsetstest_SRCS kcharsetstest.cpp )

kde4_automoc(${kcharsetstest_SRCS})

kde4_add_executable(kcharsetstest ${kcharsetstest_SRCS})

target_link_libraries(kcharsetstest  ${QT_AND_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-kcharsetstest ${EXECUTABLE_OUTPUT_PATH}/kcharsetstest)

########### next target ###############

set(kcalendartest_SRCS kcalendartest.cpp )

kde4_automoc(${kcalendartest_SRCS})

kde4_add_executable(kcalendartest ${kcalendartest_SRCS})

target_link_libraries(kcalendartest  ${QT_AND_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-kcalendartest ${EXECUTABLE_OUTPUT_PATH}/kcalendartest)

########### next target ###############

set(kmacroexpandertest_SRCS kmacroexpandertest.cpp )

kde4_automoc(${kmacroexpandertest_SRCS})

kde4_add_executable(kmacroexpandertest ${kmacroexpandertest_SRCS})

target_link_libraries(kmacroexpandertest  ${QT_AND_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-kmacroexpandertest ${EXECUTABLE_OUTPUT_PATH}/kmacroexpandertest)

########### next target ###############

set(kshelltest_SRCS kshelltest.cpp )

kde4_automoc(${kshelltest_SRCS})

kde4_add_executable(kshelltest ${kshelltest_SRCS})

target_link_libraries(kshelltest  ${QT_AND_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-kshelltest ${EXECUTABLE_OUTPUT_PATH}/kshelltest)

########### next target ###############

set(kxerrorhandlertest_SRCS kxerrorhandlertest.cpp )

kde4_automoc(${kxerrorhandlertest_SRCS})

kde4_add_executable(kxerrorhandlertest ${kxerrorhandlertest_SRCS})

target_link_libraries(kxerrorhandlertest  ${QT_AND_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-kxerrorhandlertest ${EXECUTABLE_OUTPUT_PATH}/kxerrorhandlertest)

########### next target ###############

set(startserviceby_SRCS startserviceby.cpp )

kde4_automoc(${startserviceby_SRCS})

kde4_add_executable(startserviceby ${startserviceby_SRCS})

target_link_libraries(startserviceby  ${QT_AND_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-startserviceby ${EXECUTABLE_OUTPUT_PATH}/startserviceby)

########### next target ###############

set(kglobaltest_SRCS kglobaltest.cpp )

kde4_automoc(${kglobaltest_SRCS})

kde4_add_executable(kglobaltest ${kglobaltest_SRCS})

target_link_libraries(kglobaltest  ${QT_AND_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-kglobaltest ${EXECUTABLE_OUTPUT_PATH}/kglobaltest)

########### next target ###############

set(ktimezonestest_SRCS ktimezonestest.cpp )

kde4_automoc(${ktimezonestest_SRCS})

kde4_add_executable(ktimezonestest ${ktimezonestest_SRCS})

target_link_libraries(ktimezonestest  ${QT_AND_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-ktimezonestest ${EXECUTABLE_OUTPUT_PATH}/ktimezonestest)

########### next target ###############

set(kconfigtest_SRCS kconfigtest.cpp )

kde4_automoc(${kconfigtest_SRCS})

kde4_add_executable(kconfigtest ${kconfigtest_SRCS})

target_link_libraries(kconfigtest  ${QT_AND_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-kconfigtest ${EXECUTABLE_OUTPUT_PATH}/kconfigtest)

########### next target ###############

set(ktempfiletest_SRCS ktempfiletest.cpp )

kde4_automoc(${ktempfiletest_SRCS})

kde4_add_executable(ktempfiletest ${ktempfiletest_SRCS})

target_link_libraries(ktempfiletest  ${QT_AND_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-ktempfiletest ${EXECUTABLE_OUTPUT_PATH}/ktempfiletest)

########### next target ###############

# set(kurlmimetest_SRCS kurlmimetest.cpp )
#
# kde4_automoc(${kurlmimetest_SRCS})
#
# kde4_add_executable(kurlmimetest ${kurlmimetest_SRCS})
#
# target_link_libraries(kurlmimetest  ${QT_AND_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})
#
# add_test(kdecore-kurlmimetest ${EXECUTABLE_OUTPUT_PATH}/kurlmimetest)


########### next target ###############

set(kstdacceltest_SRCS kstdacceltest.cpp )

kde4_automoc(${kstdacceltest_SRCS})

kde4_add_executable(kstdacceltest ${kstdacceltest_SRCS})

target_link_libraries(kstdacceltest  ${QT_AND_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-kstdacceltest ${EXECUTABLE_OUTPUT_PATH}/kstdacceltest)

########### next target ###############

set(klibloadertest_SRCS klibloadertest.cpp )

kde4_automoc(${klibloadertest_SRCS})

kde4_add_executable(klibloadertest ${klibloadertest_SRCS})

target_link_libraries(klibloadertest  ${QT_AND_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-klibloadertest ${EXECUTABLE_OUTPUT_PATH}/klibloadertest)

########### next target ###############

set(kcompletiontest_SRCS kcompletiontest.cpp )

kde4_automoc(${kcompletiontest_SRCS})

kde4_add_executable(kcompletiontest ${kcompletiontest_SRCS})

target_link_libraries(kcompletiontest  ${QT_AND_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-kcompletiontest ${EXECUTABLE_OUTPUT_PATH}/kcompletiontest)

########### next target ###############

set(klockfiletest_SRCS klockfiletest.cpp )

kde4_automoc(${klockfiletest_SRCS})

kde4_add_executable(lt-klockfiletest ${klockfiletest_SRCS})

target_link_libraries(lt-klockfiletest  ${QT_AND_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-klockfiletest ${EXECUTABLE_OUTPUT_PATH}/lt-klockfiletest)

########### next target ###############

set(ktempdirtest_SRCS ktempdirtest.cpp )

kde4_automoc(${ktempdirtest_SRCS})

kde4_add_executable(ktempdirtest ${ktempdirtest_SRCS})

target_link_libraries(ktempdirtest  ${QT_AND_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-ktempdirtest ${EXECUTABLE_OUTPUT_PATH}/ktempdirtest)

########### next target ###############

set(ksharedptrtest_SRCS ksharedptrtest.cpp )

kde4_automoc(${ksharedptrtest_SRCS})

kde4_add_executable(ksharedptrtest ${ksharedptrtest_SRCS})

target_link_libraries(ksharedptrtest  ${QT_AND_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-ksharedptrtest ${EXECUTABLE_OUTPUT_PATH}/ksharedptrtest)

########### next target ###############

set(kfiltertest_SRCS kfiltertest.cpp )

kde4_automoc(${kfiltertest_SRCS})

kde4_add_executable(kfiltertest ${kfiltertest_SRCS})

target_link_libraries(kfiltertest  ${QT_AND_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-kfiltertest ${EXECUTABLE_OUTPUT_PATH}/kfiltertest)

########### next target ###############

set(kdatetimetest_SRCS dummy.cpp )

kde4_automoc(${kdatetimetest_SRCS})

kde4_add_executable(kdatetimetest ${kdatetimetest_SRCS})

target_link_libraries(kdatetimetest  ${QT_AND_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-kdatetimetest ${EXECUTABLE_OUTPUT_PATH}/kdatetimetest)

########### next target ###############

set(klibloadertestmodule_PART_SRCS klibloadertest_module.cpp )

kde4_automoc(${klibloadertestmodule_PART_SRCS})

kde4_add_plugin(klibloadertestmodule WITH_PREFIX ${klibloadertestmodule_PART_SRCS})

kde4_create_libtool_file(klibloadertestmodule /kde4/ )

target_link_libraries(klibloadertestmodule  ${QT_AND_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

install_targets(${KDE4_LIB_INSTALL_DIR}/kde4 klibloadertestmodule )

endif(KDE4_BUILD_TESTS)

########### install files ###############


kde4_footer()



include_directories( ${KDE4_KDECORE_INCLUDES} )

########### next target ###############

set(kconfigtestgui_SRCS kconfigtestgui.cpp )

kde4_automoc(${kconfigtestgui_SRCS})

kde4_add_test(kconfigtestgui ${kconfigtestgui_SRCS})

target_link_libraries(kconfigtestgui  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-kconfigtestgui ${EXECUTABLE_OUTPUT_PATH}/kconfigtestgui)

########### next target ###############

set(klocaletest_SRCS klocaletest.cpp )

kde4_automoc(${klocaletest_SRCS})

kde4_add_test(klocaletest ${klocaletest_SRCS})

target_link_libraries(klocaletest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-klocaletest ${EXECUTABLE_OUTPUT_PATH}/klocaletest)

########### next target ###############

set(klocalizedstringtest_SRCS klocalizedstringtest.cpp )

kde4_automoc(${klocalizedstringtest_SRCS})

kde4_add_test(klocalizedstringtest ${klocalizedstringtest_SRCS})

target_link_libraries(klocalizedstringtest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-klocalizedstringtest ${EXECUTABLE_OUTPUT_PATH}/klocalizedstringtest)

########### next target ###############

set(kprocesstest_SRCS kprocesstest.cpp )

kde4_automoc(${kprocesstest_SRCS})

kde4_add_test(kprocesstest ${kprocesstest_SRCS})

target_link_libraries(kprocesstest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-kprocesstest ${EXECUTABLE_OUTPUT_PATH}/kprocesstest)

########### next target ###############

set(ksimpleconfigtest_SRCS ksimpleconfigtest.cpp )

kde4_automoc(${ksimpleconfigtest_SRCS})

kde4_add_test(ksimpleconfigtest ${ksimpleconfigtest_SRCS})

target_link_libraries(ksimpleconfigtest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-ksimpleconfigtest ${EXECUTABLE_OUTPUT_PATH}/ksimpleconfigtest)

########### next target ###############

set(kstandarddirstest_SRCS kstandarddirstest.cpp )

kde4_automoc(${kstandarddirstest_SRCS})

kde4_add_test(kstandarddirstest ${kstandarddirstest_SRCS})

target_link_libraries(kstandarddirstest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-kstandarddirstest ${EXECUTABLE_OUTPUT_PATH}/kstandarddirstest)

########### next target ###############

set(kurltest_SRCS kurltest.cpp )

kde4_automoc(${kurltest_SRCS})

kde4_add_test(kurltest ${kurltest_SRCS})

target_link_libraries(kurltest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-kurltest ${EXECUTABLE_OUTPUT_PATH}/kurltest)

########### next target ###############

set(krandomsequencetest_SRCS krandomsequencetest.cpp )

kde4_automoc(${krandomsequencetest_SRCS})

kde4_add_test(krandomsequencetest ${krandomsequencetest_SRCS})

target_link_libraries(krandomsequencetest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-krandomsequencetest ${EXECUTABLE_OUTPUT_PATH}/krandomsequencetest)

########### next target ###############

set(kdebugtest_SRCS kdebugtest.cpp )

kde4_automoc(${kdebugtest_SRCS})

kde4_add_test(kdebugtest ${kdebugtest_SRCS})

target_link_libraries(kdebugtest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-kdebugtest ${EXECUTABLE_OUTPUT_PATH}/kdebugtest)

########### next target ###############

set(kstringhandlertest_SRCS kstringhandlertest.cpp )

kde4_automoc(${kstringhandlertest_SRCS})

kde4_add_test(kstringhandlertest ${kstringhandlertest_SRCS})

target_link_libraries(kstringhandlertest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-kstringhandlertest ${EXECUTABLE_OUTPUT_PATH}/kstringhandlertest)

########### next target ###############

set(kcmdlineargstest_SRCS kcmdlineargstest.cpp )

kde4_automoc(${kcmdlineargstest_SRCS})

kde4_add_test(kcmdlineargstest ${kcmdlineargstest_SRCS})

target_link_libraries(kcmdlineargstest  ${KDE4_KDECORE_LIBS})

# add_test(kdecore-kcmdlineargstest ${EXECUTABLE_OUTPUT_PATH}/kcmdlineargstest)

########### next target ###############

set(kmemtest_SRCS kmemtest.cpp )

kde4_automoc(${kmemtest_SRCS})

kde4_add_test(kmemtest ${kmemtest_SRCS})

target_link_libraries(kmemtest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-kmemtest ${EXECUTABLE_OUTPUT_PATH}/kmemtest)

########### next target ###############

set(dbuscalltest_SRCS dbuscalltest.cpp )

kde4_automoc(${dbuscalltest_SRCS})

kde4_add_test(dbuscalltest ${dbuscalltest_SRCS})

target_link_libraries(dbuscalltest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

########### next target ###############

set(cplusplustest_SRCS cplusplustest.cpp )

kde4_automoc(${cplusplustest_SRCS})

kde4_add_test(cplusplustest ${cplusplustest_SRCS})

target_link_libraries(cplusplustest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-cplusplustest ${EXECUTABLE_OUTPUT_PATH}/cplusplustest)

########### next target ###############

set(kmdcodectest_SRCS kmdcodectest.cpp )

kde4_automoc(${kmdcodectest_SRCS})

kde4_add_test(kmdcodectest ${kmdcodectest_SRCS})

target_link_libraries(kmdcodectest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-kmdcodectest ${EXECUTABLE_OUTPUT_PATH}/kmdcodectest)

########### next target ###############


set(kdatetimetest_SRCS kdatetimetest.cpp )

kde4_automoc(${kdatetimetest_SRCS})

kde4_add_test(kdatetimetest ${kdatetimetest_SRCS})

target_link_libraries(kdatetimetest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-kdatetimetest ${EXECUTABLE_OUTPUT_PATH}/kdatetimetest)

########### next target ###############

set(ksortablelisttest_SRCS ksortablelisttest.cpp )

kde4_automoc(${ksortablelisttest_SRCS})

kde4_add_test(ksortablelisttest ${ksortablelisttest_SRCS})

target_link_libraries(ksortablelisttest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-ksortablelisttest ${EXECUTABLE_OUTPUT_PATH}/ksortablelisttest)

########### next target ###############

set(krfcdatetest_SRCS krfcdatetest.cpp )

kde4_automoc(${krfcdatetest_SRCS})

kde4_add_test(krfcdatetest ${krfcdatetest_SRCS})

target_link_libraries(krfcdatetest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-krfcdatetest ${EXECUTABLE_OUTPUT_PATH}/krfcdatetest)

########### next target ###############

set(kprociotest_SRCS kprociotest.cpp )

kde4_automoc(${kprociotest_SRCS})

kde4_add_test(kprociotest ${kprociotest_SRCS})

target_link_libraries(kprociotest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-kprociotest ${EXECUTABLE_OUTPUT_PATH}/kprociotest)

########### next target ###############

set(kcharsetstest_SRCS kcharsetstest.cpp )

kde4_automoc(${kcharsetstest_SRCS})

kde4_add_test(kcharsetstest ${kcharsetstest_SRCS})

target_link_libraries(kcharsetstest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-kcharsetstest ${EXECUTABLE_OUTPUT_PATH}/kcharsetstest)

########### next target ###############

set(kcalendartest_SRCS kcalendartest.cpp )

kde4_automoc(${kcalendartest_SRCS})

kde4_add_test(kcalendartest ${kcalendartest_SRCS})

target_link_libraries(kcalendartest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-kcalendartest ${EXECUTABLE_OUTPUT_PATH}/kcalendartest)

########### next target ###############

set(kmacroexpandertest_SRCS kmacroexpandertest.cpp )

kde4_automoc(${kmacroexpandertest_SRCS})

kde4_add_test(kmacroexpandertest ${kmacroexpandertest_SRCS})

target_link_libraries(kmacroexpandertest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-kmacroexpandertest ${EXECUTABLE_OUTPUT_PATH}/kmacroexpandertest)

########### next target ###############

set(kshelltest_SRCS kshelltest.cpp )

kde4_automoc(${kshelltest_SRCS})

kde4_add_test(kshelltest ${kshelltest_SRCS})

target_link_libraries(kshelltest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-kshelltest ${EXECUTABLE_OUTPUT_PATH}/kshelltest)

########### next target ###############

set(startserviceby_SRCS startserviceby.cpp )

kde4_automoc(${startserviceby_SRCS})

kde4_add_test(startserviceby ${startserviceby_SRCS})

target_link_libraries(startserviceby  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-startserviceby ${EXECUTABLE_OUTPUT_PATH}/startserviceby)

########### next target ###############

set(kasciitest_SRCS kasciitest.cpp )

kde4_automoc(${kasciitest_SRCS})

kde4_add_test(kasciitest ${kasciitest_SRCS})

target_link_libraries(kasciitest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-kasciitest ${EXECUTABLE_OUTPUT_PATH}/kasciitest)

########### next target ###############

set(ktimezonestest_SRCS ktimezonestest.cpp )

kde4_automoc(${ktimezonestest_SRCS})

kde4_add_test(ktimezonestest ${ktimezonestest_SRCS})

target_link_libraries(ktimezonestest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-ktimezonestest ${EXECUTABLE_OUTPUT_PATH}/ktimezonestest)

########### next target ###############

set(kconfigtest_SRCS kconfigtest.cpp )

kde4_automoc(${kconfigtest_SRCS})

kde4_add_test(kconfigtest ${kconfigtest_SRCS})

target_link_libraries(kconfigtest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-kconfigtest ${EXECUTABLE_OUTPUT_PATH}/kconfigtest)

########### next target ###############

set(kconfigskeletontest_SRCS kconfigskeletontest.cpp )

kde4_automoc(${kconfigskeletontest_SRCS})

kde4_add_test(kconfigskeletontest ${kconfigskeletontest_SRCS})

target_link_libraries(kconfigskeletontest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-kconfigskeletontest ${EXECUTABLE_OUTPUT_PATH}/kconfigskeletontest)

########### next target ###############

set(kurlmimetest_SRCS kurlmimetest.cpp )

kde4_automoc(${kurlmimetest_SRCS})

kde4_add_test(kurlmimetest ${kurlmimetest_SRCS})

target_link_libraries(kurlmimetest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-kurlmimetest ${EXECUTABLE_OUTPUT_PATH}/kurlmimetest)

########### next target ###############


set(klockfiletest_SRCS klockfiletest.cpp )

kde4_automoc(${klockfiletest_SRCS})

kde4_add_test(klockfiletest ${klockfiletest_SRCS})

target_link_libraries(klockfiletest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-klockfiletest ${EXECUTABLE_OUTPUT_PATH}/klockfiletest)

########### next target ###############

set(ktempdirtest_SRCS ktempdirtest.cpp )

kde4_automoc(${ktempdirtest_SRCS})

kde4_add_test(ktempdirtest ${ktempdirtest_SRCS})

target_link_libraries(ktempdirtest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-ktempdirtest ${EXECUTABLE_OUTPUT_PATH}/ktempdirtest)

########### next target ###############

set(ksharedptrtest_SRCS ksharedptrtest.cpp )

kde4_automoc(${ksharedptrtest_SRCS})

kde4_add_test(ksharedptrtest ${ksharedptrtest_SRCS})

target_link_libraries(ksharedptrtest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-ksharedptrtest ${EXECUTABLE_OUTPUT_PATH}/ksharedptrtest)

########### next target ###############

set(kfiltertest_SRCS kfiltertest.cpp )

kde4_automoc(${kfiltertest_SRCS})

kde4_add_test(kfiltertest ${kfiltertest_SRCS})

target_link_libraries(kfiltertest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-kfiltertest ${EXECUTABLE_OUTPUT_PATH}/kfiltertest)

########### next target ###############

set(kdatetimetest_SRCS dummy.cpp )

kde4_automoc(${kdatetimetest_SRCS})

kde4_add_test(kdatetimetest ${kdatetimetest_SRCS})

target_link_libraries(kdatetimetest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-kdatetimetest ${EXECUTABLE_OUTPUT_PATH}/kdatetimetest)

########### next target ###############

set(ksavefiletest_SRCS ksavefiletest.cpp )

kde4_automoc(${ksavefiletest_SRCS})

kde4_add_test(ksavefiletest ${ksavefiletest_SRCS})

target_link_libraries(ksavefiletest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-ksavefiletest ${EXECUTABLE_OUTPUT_PATH}/ksavefiletest)

########### next target ###############

set(kautosavefiletest_SRCS kautosavefiletest.cpp )

kde4_automoc(${kautosavefiletest_SRCS})

kde4_add_test(kautosavefiletest ${kautosavefiletest_SRCS})

target_link_libraries(kautosavefiletest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-kautosavefiletest ${EXECUTABLE_OUTPUT_PATH}/kautosavefiletest)

if (KDE4_BUILD_TESTS)
   ########### next target ###############

   set(klibloadertest_SRCS klibloadertest.cpp )

   kde4_automoc(${klibloadertest_SRCS})

   kde4_add_test(klibloadertest ${klibloadertest_SRCS})

   target_link_libraries(klibloadertest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

   add_test(kdecore-klibloadertest ${EXECUTABLE_OUTPUT_PATH}/klibloadertest)

   ########### next target ###############

   set(klibloadertestmodule_PART_SRCS klibloadertest_module.cpp )

   kde4_automoc(${klibloadertestmodule_PART_SRCS})

   kde4_add_plugin(klibloadertestmodule WITH_PREFIX ${klibloadertestmodule_PART_SRCS})

   target_link_libraries(klibloadertestmodule  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

   ########### next target ###############

   set(klibloadertestmodule4_PART_SRCS klibloadertest4_module.cpp )

   kde4_automoc(${klibloadertestmodule4_PART_SRCS})

   kde4_add_plugin(klibloadertestmodule4 WITH_PREFIX ${klibloadertestmodule4_PART_SRCS})

   target_link_libraries(klibloadertestmodule4  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

   # hack - we don't really want to install this module [it would break later runs when the module is modified locally]
   # so we do the .la by hand
   #install(TARGETS klibloadertestmodule  DESTINATION ${PLUGIN_INSTALL_DIR} )

   set(_laname klibloadertestmodule4)
       GET_TARGET_PROPERTY(_target_location klibloadertestmodule4 LOCATION)
       GET_FILENAME_COMPONENT(_laname ${_target_location} NAME_WE)
       GET_FILENAME_COMPONENT(_soname ${_target_location} NAME)
   set(_laname ${LIBRARY_OUTPUT_PATH}/${_laname}.la)
       FILE(WRITE ${_laname} "dlname='${_soname}'\n")
       FILE(APPEND ${_laname} "library_names='${_soname} ${_soname} ${_soname}'\n")

   set_target_properties(klibloadertestmodule4 PROPERTIES SKIP_BUILD_RPATH FALSE BUILD_WITH_INSTALL_RPATH FALSE)

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(ktemporaryfiletest_SRCS ktemporaryfiletest.cpp )

kde4_automoc(${ktemporaryfiletest_SRCS})

kde4_add_test(ktemporaryfiletest ${ktemporaryfiletest_SRCS})

target_link_libraries(ktemporaryfiletest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-ktemporaryfiletest ${EXECUTABLE_OUTPUT_PATH}/ktemporaryfiletest)

########### next target ###############

set(kautostarttest_SRCS kautostarttest.cpp )

kde4_automoc(${kautostarttest_SRCS})

kde4_add_test(kautostarttest ${kautostarttest_SRCS})

target_link_libraries(kautostarttest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-kautostarttest ${EXECUTABLE_OUTPUT_PATH}/kautostarttest)

########### next target ###############

set(kjobtest_SRCS kjobtest.cpp )

kde4_automoc(${kjobtest_SRCS})

kde4_add_test(kjobtest ${kjobtest_SRCS})

target_link_libraries(kjobtest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-kjobtest ${EXECUTABLE_OUTPUT_PATH}/kjobtest)

########### kservicetest ###############

set(kservicetest_SRCS kservicetest.cpp )

kde4_automoc(${kservicetest_SRCS})

kde4_add_test(kservicetest ${kservicetest_SRCS})

target_link_libraries(kservicetest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-kservicetest ${EXECUTABLE_OUTPUT_PATH}/kservicetest)

########### kglobalstatictest ###############

set(kglobalstatictest_SRCS kglobalstatictest.cpp )

kde4_automoc(${kglobalstatictest_SRCS})

kde4_add_executable(kglobalstatictest NOGUI RUN_UNINSTALLED ${kglobalstatictest_SRCS})

target_link_libraries(kglobalstatictest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-kglobalstatictest ${EXECUTABLE_OUTPUT_PATH}/kglobalstatictest)

#######################################


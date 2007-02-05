
if(KDE4_BUILD_TESTS)

# Tests don't need to go into toplevel/bin, they are fine in the current dir.
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

# This can be used for finding .rc files in the source dir, without installing them
add_definitions( -DKDESRCDIR=\\"${CMAKE_CURRENT_SOURCE_DIR}\\" )

include_directories( ${KDE4_KDEUI_INCLUDES} )


########### next target ###############

set(kblendtest_SRCS kblendtest.cpp)

kde4_automoc(${kblendtest_SRCS})

kde4_add_executable(kblendtest NOGUI RUN_UNINSTALLED ${kblendtest_SRCS})

target_link_libraries(kblendtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kdialogbuttonboxtest_SRCS
kdialogbuttonboxtest.cpp
)

kde4_automoc(${kdialogbuttonboxtest_SRCS})

kde4_add_executable(kdialogbuttonboxtest NOGUI RUN_UNINSTALLED ${kdialogbuttonboxtest_SRCS})

target_link_libraries(kdialogbuttonboxtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kcharselecttest_SRCS
kcharselecttest.cpp
)

kde4_automoc(${kcharselecttest_SRCS})

kde4_add_executable(kcharselecttest NOGUI RUN_UNINSTALLED ${kcharselecttest_SRCS})

target_link_libraries(kcharselecttest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kcolortest_SRCS
kcolortest.cpp
)

kde4_automoc(${kcolortest_SRCS})

kde4_add_executable(kcolortest NOGUI RUN_UNINSTALLED ${kcolortest_SRCS})

target_link_libraries(kcolortest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kcolordlgtest_SRCS
kcolordlgtest.cpp
)

kde4_automoc(${kcolordlgtest_SRCS})

kde4_add_executable(kcolordlgtest NOGUI RUN_UNINSTALLED ${kcolordlgtest_SRCS})

target_link_libraries(kcolordlgtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kcomboboxtest_SRCS
kcomboboxtest.cpp
)

kde4_automoc(${kcomboboxtest_SRCS})

kde4_add_executable(kcomboboxtest NOGUI RUN_UNINSTALLED ${kcomboboxtest_SRCS})

target_link_libraries(kcomboboxtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kcompletiontest_kdeui_SRCS
kcompletionuitest.cpp
)

kde4_automoc(${kcompletiontest_kdeui_SRCS})

kde4_add_executable(kcompletiontest_kdeui NOGUI RUN_UNINSTALLED ${kcompletiontest_kdeui_SRCS})

target_link_libraries(kcompletiontest_kdeui ${KDE4_KDEUI_LIBS})




set(kcompletiontest_SRCS kcompletioncoretest.cpp )

kde4_automoc(${kcompletiontest_SRCS})

kde4_add_executable(kcompletiontest NOGUI RUN_UNINSTALLED ${kcompletiontest_SRCS})

target_link_libraries(kcompletiontest  ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kcompletiontest ${EXECUTABLE_OUTPUT_PATH}/kcompletiontest)

########### kmainwindow_unittest ###############

set(kmainwindow_unittest_SRCS kmainwindow_unittest.cpp )

kde4_automoc(${kmainwindow_unittest_SRCS})

kde4_add_executable(kmainwindow_unittest NOGUI RUN_UNINSTALLED ${kmainwindow_unittest_SRCS})

target_link_libraries(kmainwindow_unittest  ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kmainwindow_unittest ${EXECUTABLE_OUTPUT_PATH}/kmainwindow_unittest)

########### next target ###############

set(kdatepicktest_SRCS
kdatepicktest.cpp
)

kde4_automoc(${kdatepicktest_SRCS})

kde4_add_executable(kdatepicktest NOGUI RUN_UNINSTALLED ${kdatepicktest_SRCS})

target_link_libraries(kdatepicktest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kdatewidgettest_SRCS
kdatewidgettest.cpp
)

kde4_automoc(${kdatewidgettest_SRCS})

kde4_add_executable(kdatewidgettest NOGUI RUN_UNINSTALLED ${kdatewidgettest_SRCS})

target_link_libraries(kdatewidgettest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(keditlistboxtest_SRCS
keditlistboxtest.cpp
)

kde4_automoc(${keditlistboxtest_SRCS})

kde4_add_executable(keditlistboxtest NOGUI RUN_UNINSTALLED ${keditlistboxtest_SRCS})

target_link_libraries(keditlistboxtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kfontdialogtest_SRCS
kfontdialogtest.cpp
)

kde4_automoc(${kfontdialogtest_SRCS})

kde4_add_executable(kfontdialogtest NOGUI RUN_UNINSTALLED ${kfontdialogtest_SRCS})

target_link_libraries(kfontdialogtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kgradienttest_SRCS
kgradienttest.cpp
)

kde4_automoc(${kgradienttest_SRCS})

kde4_add_executable(kgradienttest NOGUI RUN_UNINSTALLED ${kgradienttest_SRCS})

target_link_libraries(kgradienttest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(khashtest_SRCS
khashtest.cpp
)

kde4_automoc(${khashtest_SRCS})

kde4_add_executable(khashtest NOGUI RUN_UNINSTALLED ${khashtest_SRCS})

target_link_libraries(khashtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kdesattest_SRCS
kdesattest.cpp
)

kde4_automoc(${kdesattest_SRCS})

kde4_add_executable(kdesattest NOGUI RUN_UNINSTALLED ${kdesattest_SRCS})

target_link_libraries(kdesattest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kledtest_SRCS
kledtest.cpp
)

kde4_automoc(${kledtest_SRCS})

kde4_add_executable(kledtest NOGUI RUN_UNINSTALLED ${kledtest_SRCS})

target_link_libraries(kledtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(klineedittest_SRCS
klineedittest.cpp
)

kde4_automoc(${klineedittest_SRCS})

kde4_add_executable(klineedittest NOGUI RUN_UNINSTALLED ${klineedittest_SRCS})

target_link_libraries(klineedittest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kmessageboxtest_SRCS
kmessageboxtest.cpp
)

kde4_automoc(${kmessageboxtest_SRCS})

kde4_add_executable(kmessageboxtest NOGUI RUN_UNINSTALLED ${kmessageboxtest_SRCS})

target_link_libraries(kmessageboxtest ${KDE4_KDEUI_LIBS})

########### next target ###############

set(knuminputtest_SRCS
knuminputtest.cpp
)

kde4_automoc(${knuminputtest_SRCS})

kde4_add_executable(knuminputtest NOGUI RUN_UNINSTALLED ${knuminputtest_SRCS})

target_link_libraries(knuminputtest ${KDE4_KDEUI_LIBS})


########### next target ###############
set(kpalettetest_SRCS
    kpalettetest.cpp
)

kde4_automoc(${kpalettetest_SRCS})

kde4_add_executable(kpalettetest NOGUI RUN_UNINSTALLED ${kpalettetest_SRCS})

target_link_libraries(kpalettetest ${KDE4_KDEUI_LIBS})

########### next target ###############

set(kpopuptest_SRCS
kpopuptest.cpp
)

kde4_automoc(${kpopuptest_SRCS})

kde4_add_executable(kpopuptest NOGUI RUN_UNINSTALLED ${kpopuptest_SRCS})

target_link_libraries(kpopuptest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kprogressdialogtest_SRCS
kprogressdialogtest.cpp
)

kde4_automoc(${kprogressdialogtest_SRCS})

kde4_add_executable(kprogressdialogtest NOGUI RUN_UNINSTALLED ${kprogressdialogtest_SRCS})

target_link_libraries(kprogressdialogtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(krulertest_SRCS
krulertest.cpp
)

kde4_automoc(${krulertest_SRCS})

kde4_add_executable(krulertest NOGUI RUN_UNINSTALLED ${krulertest_SRCS})

target_link_libraries(krulertest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kseparatortest_SRCS
kseparatortest.cpp
)

kde4_automoc(${kseparatortest_SRCS})

kde4_add_executable(kseparatortest NOGUI RUN_UNINSTALLED ${kseparatortest_SRCS})

target_link_libraries(kseparatortest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kstatusbartest_SRCS
kstatusbartest.cpp
)

kde4_automoc(${kstatusbartest_SRCS})

kde4_add_executable(kstatusbartest NOGUI RUN_UNINSTALLED ${kstatusbartest_SRCS})

target_link_libraries(kstatusbartest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kunbalancedgrdtest_SRCS
kunbalancedgrdtest.cpp
)

kde4_automoc(${kunbalancedgrdtest_SRCS})

kde4_add_executable(kunbalancedgrdtest NOGUI RUN_UNINSTALLED ${kunbalancedgrdtest_SRCS})

target_link_libraries(kunbalancedgrdtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kwindowtest_SRCS
kwindowtest.cpp
)

kde4_automoc(${kwindowtest_SRCS})

kde4_add_executable(kwindowtest NOGUI RUN_UNINSTALLED ${kwindowtest_SRCS})

target_link_libraries(kwindowtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kselectactiontest_SRCS
kselectactiontest.cpp
)

kde4_automoc(${kselectactiontest_SRCS})

kde4_add_executable(kselectactiontest NOGUI RUN_UNINSTALLED ${kselectactiontest_SRCS})

target_link_libraries(kselectactiontest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(itemcontainertest_SRCS
itemcontainertest.cpp
)

kde4_automoc(${itemcontainertest_SRCS})

kde4_add_executable(itemcontainertest NOGUI RUN_UNINSTALLED ${itemcontainertest_SRCS})

target_link_libraries(itemcontainertest ${KDE4_KDEUI_LIBS})

########### next target ###############

set(kxmlguitest_SRCS
kxmlguitest.cpp
)

kde4_automoc(${kxmlguitest_SRCS})

kde4_add_executable(kxmlguitest NOGUI RUN_UNINSTALLED ${kxmlguitest_SRCS})

target_link_libraries(kxmlguitest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kaccelgentest_SRCS
kaccelgentest.cpp
)

kde4_automoc(${kaccelgentest_SRCS})

kde4_add_executable(kaccelgentest NOGUI RUN_UNINSTALLED ${kaccelgentest_SRCS})

target_link_libraries(kaccelgentest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(ksystemtraytest_SRCS
ksystemtraytest.cpp
)

kde4_automoc(${ksystemtraytest_SRCS})

kde4_add_executable(ksystemtraytest NOGUI RUN_UNINSTALLED ${ksystemtraytest_SRCS})

target_link_libraries(ksystemtraytest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(k3listviewtest_SRCS
k3listviewtest.cpp
)

kde4_automoc(${k3listviewtest_SRCS})

kde4_add_executable(k3listviewtest NOGUI RUN_UNINSTALLED ${k3listviewtest_SRCS})

target_link_libraries(k3listviewtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(ktreewidgetsearchlinetest_SRCS
ktreewidgetsearchlinetest.cpp
)

kde4_automoc(${ktreewidgetsearchlinetest_SRCS})

kde4_add_executable(ktreewidgetsearchlinetest NOGUI RUN_UNINSTALLED ${ktreewidgetsearchlinetest_SRCS})

target_link_libraries(ktreewidgetsearchlinetest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(ktextedittest_SRCS
ktextedittest.cpp
)

kde4_automoc(${ktextedittest_SRCS})

kde4_add_executable(ktextedittest NOGUI RUN_UNINSTALLED ${ktextedittest_SRCS})

target_link_libraries(ktextedittest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kmainwindowtest_SRCS
kmainwindowtest.cpp
)

kde4_automoc(${kmainwindowtest_SRCS})

kde4_add_executable(kmainwindowtest NOGUI RUN_UNINSTALLED ${kmainwindowtest_SRCS})

target_link_libraries(kmainwindowtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kmainwindowrestoretest_SRCS
kmainwindowrestoretest.cpp
)

kde4_automoc(${kmainwindowrestoretest_SRCS})

kde4_add_executable(kmainwindowrestoretest NOGUI RUN_UNINSTALLED ${kmainwindowrestoretest_SRCS})

target_link_libraries(kmainwindowrestoretest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kactioncollectiontest_SRCS
kactioncollectiontest.cpp
)

kde4_automoc(${kactioncollectiontest_SRCS})

kde4_add_executable(kactioncollectiontest NOGUI RUN_UNINSTALLED ${kactioncollectiontest_SRCS})

target_link_libraries(kactioncollectiontest ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kstandardactiontest_SRCS
kstandardactiontest.cpp
)

kde4_automoc(${kstandardactiontest_SRCS})

kde4_add_executable(kstandardactiontest NOGUI RUN_UNINSTALLED ${kstandardactiontest_SRCS})

target_link_libraries(kstandardactiontest ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY})


########### next target ###############

set(kcodecactiontest_SRCS
kcodecactiontest.cpp
)

kde4_automoc(kcodecactiontest ${kcodecactiontest_SRCS})

kde4_add_executable(kcodecactiontest NOGUI RUN_UNINSTALLED ${kcodecactiontest_SRCS})

target_link_libraries(kcodecactiontest ${KDE4_KDEUI_LIBS})

########### next target ###############

set(ktoolbartest_SRCS
ktoolbartest.cpp
)

kde4_automoc(${ktoolbartest_SRCS})

kde4_add_executable(ktoolbartest NOGUI RUN_UNINSTALLED ${ktoolbartest_SRCS})

target_link_libraries(ktoolbartest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kdatetimewidgettest_SRCS
kdatetimewidgettest.cpp
)

kde4_automoc(${kdatetimewidgettest_SRCS})

kde4_add_executable(kdatetimewidgettest NOGUI RUN_UNINSTALLED ${kdatetimewidgettest_SRCS})

target_link_libraries(kdatetimewidgettest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kdatetabletest_SRCS
kdatetabletest.cpp
)

kde4_automoc(${kdatetabletest_SRCS})

kde4_add_executable(kdatetabletest NOGUI RUN_UNINSTALLED ${kdatetabletest_SRCS})

target_link_libraries(kdatetabletest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kinputdialogtest_SRCS
kinputdialogtest.cpp
)

kde4_automoc(${kinputdialogtest_SRCS})

kde4_add_executable(kinputdialogtest NOGUI RUN_UNINSTALLED ${kinputdialogtest_SRCS})

target_link_libraries(kinputdialogtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(ktabwidgettest_SRCS
ktabwidgettest.cpp
)

kde4_automoc(${ktabwidgettest_SRCS})

kde4_add_executable(ktabwidgettest NOGUI RUN_UNINSTALLED ${ktabwidgettest_SRCS})

target_link_libraries(ktabwidgettest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kwordwraptest_SRCS
kwordwraptest.cpp
)

kde4_automoc(${kwordwraptest_SRCS})

kde4_add_executable(kwordwraptest NOGUI RUN_UNINSTALLED ${kwordwraptest_SRCS})

target_link_libraries(kwordwraptest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(ksqueezedtextlabeltest_SRCS
ksqueezedtextlabeltest.cpp
)

kde4_automoc(${ksqueezedtextlabeltest_SRCS})

kde4_add_executable(ksqueezedtextlabeltest NOGUI RUN_UNINSTALLED ${ksqueezedtextlabeltest_SRCS})

target_link_libraries(ksqueezedtextlabeltest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kpixmapregionselectordialogtest_SRCS
kpixmapregionselectordialogtest.cpp
)

kde4_automoc(${kpixmapregionselectordialogtest_SRCS})

kde4_add_executable(kpixmapregionselectordialogtest NOGUI RUN_UNINSTALLED ${kpixmapregionselectordialogtest_SRCS})

target_link_libraries(kpixmapregionselectordialogtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(ktoolbarlabelactiontest_SRCS
ktoolbarlabelactiontest.cpp
)

kde4_automoc(${ktoolbarlabelactiontest_SRCS})

kde4_add_executable(ktoolbarlabelactiontest NOGUI RUN_UNINSTALLED ${ktoolbarlabelactiontest_SRCS})

target_link_libraries(ktoolbarlabelactiontest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(khboxtest_SRCS
khboxtest.cpp
)

kde4_automoc(${khboxtest_SRCS})

kde4_add_executable(khboxtest NOGUI RUN_UNINSTALLED ${khboxtest_SRCS})

target_link_libraries(khboxtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kbugreporttest_SRCS
kbugreporttest.cpp
)

kde4_automoc(${kbugreporttest_SRCS})

kde4_add_executable(kbugreporttest NOGUI RUN_UNINSTALLED ${kbugreporttest_SRCS})

target_link_libraries(kbugreporttest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kdialogtest_SRCS
kdialogtest.cpp
)

kde4_automoc(${kdialogtest_SRCS})

kde4_add_executable(kdialogtest NOGUI RUN_UNINSTALLED ${kdialogtest_SRCS})

target_link_libraries(kdialogtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kglobalacceltest_SRCS
kglobalacceltest.cpp
)

kde4_automoc(${kglobalacceltest_SRCS})

kde4_add_executable(kglobalacceltest NOGUI RUN_UNINSTALLED ${kglobalacceltest_SRCS})

target_link_libraries(kglobalacceltest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(knotificationrestrictionstest_SRCS
knotificationrestrictionstest.cpp
)

kde4_automoc(${knotificationrestrictionstest_SRCS})

kde4_add_executable(knotificationrestrictionstest NOGUI RUN_UNINSTALLED ${knotificationrestrictionstest_SRCS})

target_link_libraries(knotificationrestrictionstest  ${KDE4_KDEUI_LIBS})

########### next target ###############

set(kmessagetest_SRCS
kmessagetest.cpp
)

kde4_automoc(${kmessagetest_SRCS})

kde4_add_executable(kmessagetest NOGUI RUN_UNINSTALLED ${kmessagetest_SRCS})

target_link_libraries(kmessagetest ${KDE4_KDEUI_LIBS})

########### next target ###############
set(kpagedialogtest_SRCS
kpagedialogtest.cpp
)

kde4_automoc(${kpagedialogtest_SRCS})

kde4_add_executable(kpagedialogtest NOGUI RUN_UNINSTALLED ${kpagedialogtest_SRCS})

target_link_libraries(kpagedialogtest  ${KDE4_KDEUI_LIBS})

########### next target ###############
set(kpagewidgettest_SRCS
kpagewidgettest.cpp
)

kde4_automoc(${kpagewidgettest_SRCS})

kde4_add_executable(kpagewidgettest NOGUI RUN_UNINSTALLED ${kpagewidgettest_SRCS})

target_link_libraries(kpagewidgettest  ${KDE4_KDEUI_LIBS})

########### next target ###############

set(kfindtest_SRCS kfindtest.cpp )

kde4_automoc(${kfindtest_SRCS})

kde4_add_executable(kfindtest NOGUI ${kfindtest_SRCS})

target_link_libraries(kfindtest ${KDE4_KDEUI_LIBS} )

########### next target ###############

set(kreplacetest_SRCS kreplacetest.cpp )

kde4_automoc(${kreplacetest_SRCS})

kde4_add_executable(kreplacetest NOGUI ${kreplacetest_SRCS})

target_link_libraries(kreplacetest  ${KDE4_KDEUI_LIBS} )

########### kbuttongrouptest ###############

set(kbuttongrouptest_SRCS kbuttongrouptest.cpp )

kde4_automoc(${kbuttongrouptest_SRCS})

kde4_add_executable(kbuttongrouptest NOGUI RUN_UNINSTALLED ${kbuttongrouptest_SRCS})

target_link_libraries(kbuttongrouptest  ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kbuttongrouptest ${EXECUTABLE_OUTPUT_PATH}/kbuttongrouptest)


########### kassistantdialogtest ###############

set(kassistantdialogtest_SRCS kassistantdialogtest.cpp )

kde4_automoc(${kassistantdialogtest_SRCS})

kde4_add_executable(kassistantdialogtest NOGUI RUN_UNINSTALLED ${kassistantdialogtest_SRCS})

target_link_libraries(kassistantdialogtest ${KDE4_KDEUI_LIBS})

########### kactionselectortest ###############

set(kactionselectortest_SRCS kactionselectortest.cpp )

kde4_automoc(${kactionselectortest_SRCS})

kde4_add_executable(kactionselectortest NOGUI RUN_UNINSTALLED ${kactionselectortest_SRCS})

target_link_libraries(kactionselectortest ${KDE4_KDEUI_LIBS})


########### kcommandtest ###############

set(kcommandtest_SRCS kcommandtest.cpp )

kde4_automoc(kcommandtest ${kcommandtest_SRCS})

kde4_add_executable(kcommandtest NOGUI RUN_UNINSTALLED ${kcommandtest_SRCS})

target_link_libraries(kcommandtest  ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kcommandtest ${EXECUTABLE_OUTPUT_PATH}/kcommandtest)


########### next target ###############

set(kapptest_SRCS kapptest.cpp )

kde4_automoc(${kapptest_SRCS})

kde4_add_executable(kapptest NOGUI RUN_UNINSTALLED ${kapptest_SRCS})

target_link_libraries(kapptest  ${KDE4_KDEUI_LIBS})

# add_test(kdeui-kapptest ${EXECUTABLE_OUTPUT_PATH}/kapptest)

########### next target ###############

set(kuniqueapptest_SRCS kuniqueapptest.cpp )

kde4_automoc(${kuniqueapptest_SRCS})

kde4_add_executable(kuniqueapptest NOGUI RUN_UNINSTALLED ${kuniqueapptest_SRCS})

target_link_libraries(kuniqueapptest  ${KDE4_KDEUI_LIBS} )

add_test(kdeui-kuniqueapptest ${EXECUTABLE_OUTPUT_PATH}/kuniqueapptest)

########### next target ###############

set(testqtargs_SRCS testqtargs.cpp )

kde4_automoc(${testqtargs_SRCS})

kde4_add_executable(testqtargs NOGUI RUN_UNINSTALLED ${testqtargs_SRCS})

target_link_libraries(testqtargs  ${KDE4_KDEUI_LIBS})

# add_test(kdeui-testqtargs ${EXECUTABLE_OUTPUT_PATH}/testqtargs)

########### next target ###############

set(kiconloadertest_SRCS kiconloadertest.cpp )

kde4_automoc(${kiconloadertest_SRCS})

kde4_add_executable(kiconloadertest NOGUI RUN_UNINSTALLED ${kiconloadertest_SRCS})

target_link_libraries(kiconloadertest ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdeui-kiconloadertest ${EXECUTABLE_OUTPUT_PATH}/kiconloadertest)

########### next target ###############

set(kstandardshortcuttest_SRCS kstandardshortcuttest.cpp )

kde4_automoc(${kstandardshortcuttest_SRCS})

kde4_add_executable(kstandardshortcuttest NOGUI RUN_UNINSTALLED ${kstandardshortcuttest_SRCS})

target_link_libraries(kstandardshortcuttest  ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdeui-kstandardshortcuttest ${EXECUTABLE_OUTPUT_PATH}/kstandardshortcuttest)


########### next target ###############

if (Q_WS_X11)
set(kxerrorhandlertest_SRCS kxerrorhandlertest.cpp )

kde4_automoc(${kxerrorhandlertest_SRCS})

kde4_add_executable(kxerrorhandlertest NOGUI RUN_UNINSTALLED ${kxerrorhandlertest_SRCS})

target_link_libraries(kxerrorhandlertest  ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY})

# add_test(kdecore-kxerrorhandlertest ${EXECUTABLE_OUTPUT_PATH}/kxerrorhandlertest)
endif (Q_WS_X11)

########### kglobalsettingsclient, helper program for kglobalsettingstest ###############

set(kglobalsettingsclient_SRCS kglobalsettingsclient.cpp )

kde4_automoc(${kglobalsettingsclient_SRCS})

kde4_add_executable(kglobalsettingsclient NOGUI RUN_UNINSTALLED ${kglobalsettingsclient_SRCS})

target_link_libraries(kglobalsettingsclient ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS})

########### kglobalsettingstest ###############

set(kglobalsettingstest_SRCS kglobalsettingstest.cpp )

kde4_automoc(${kglobalsettingstest_SRCS})

kde4_add_executable(kglobalsettingstest NOGUI RUN_UNINSTALLED ${kglobalsettingstest_SRCS})

target_link_libraries(kglobalsettingstest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS})

add_test(kdecore-kglobalsettingstest ${EXECUTABLE_OUTPUT_PATH}/kglobalsettingstest)

########### next target ###############

set(knewpassworddialogtest_SRCS
knewpassworddialogtest.cpp
)

kde4_automoc(${knewpassworddialogtest_SRCS})

kde4_add_executable(knewpassworddialogtest NOGUI RUN_UNINSTALLED ${knewpassworddialogtest_SRCS})

target_link_libraries(knewpassworddialogtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kpassworddialogtest_SRCS
kpassworddialogtest.cpp
)

kde4_automoc(${kpassworddialogtest_SRCS})

kde4_add_executable(kpassworddialogtest NOGUI RUN_UNINSTALLED ${kpassworddialogtest_SRCS})

target_link_libraries(kpassworddialogtest ${KDE4_KDEUI_LIBS})



set(kshortcuttest_SRCS kshortcuttest.cpp )

kde4_automoc(${kshortcuttest_SRCS})

kde4_add_executable(kshortcuttest NOGUI RUN_UNINSTALLED ${kshortcuttest_SRCS})

target_link_libraries(kshortcuttest  ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kdecore-kshortcuttest ${EXECUTABLE_OUTPUT_PATH}/kshortcuttest)

########### End of tests ###############

endif(KDE4_BUILD_TESTS)

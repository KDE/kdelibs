
if(KDE4_BUILD_TESTS)

# Tests don't need to go into toplevel/bin, they are fine in the current dir.
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

# This can be used for finding .rc files in the source dir, without installing them
add_definitions( -DKDESRCDIR=\\"${CMAKE_CURRENT_SOURCE_DIR}\\" )

include_directories( ${KDE4_KDEUI_INCLUDES} )


########### next target ###############

set(kaboutdialogtest_SRCS kaboutdialogtest.cpp)

kde4_automoc(${kaboutdialogtest_SRCS})

kde4_add_executable(kaboutdialogtest RUN_UNINSTALLED ${kaboutdialogtest_SRCS})

target_link_libraries(kaboutdialogtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kblendtest_SRCS kblendtest.cpp)

kde4_automoc(${kblendtest_SRCS})

kde4_add_executable(kblendtest RUN_UNINSTALLED ${kblendtest_SRCS})

target_link_libraries(kblendtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kdialogbuttonboxtest_SRCS
kdialogbuttonboxtest.cpp
)

kde4_automoc(${kdialogbuttonboxtest_SRCS})

kde4_add_executable(kdialogbuttonboxtest RUN_UNINSTALLED ${kdialogbuttonboxtest_SRCS})

target_link_libraries(kdialogbuttonboxtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kcharselecttest_SRCS
kcharselecttest.cpp
)

kde4_automoc(${kcharselecttest_SRCS})

kde4_add_executable(kcharselecttest RUN_UNINSTALLED ${kcharselecttest_SRCS})

target_link_libraries(kcharselecttest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kcolortest_SRCS
kcolortest.cpp
)

kde4_automoc(${kcolortest_SRCS})

kde4_add_executable(kcolortest RUN_UNINSTALLED ${kcolortest_SRCS})

target_link_libraries(kcolortest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kcolordlgtest_SRCS
kcolordlgtest.cpp
)

kde4_automoc(${kcolordlgtest_SRCS})

kde4_add_executable(kcolordlgtest RUN_UNINSTALLED ${kcolordlgtest_SRCS})

target_link_libraries(kcolordlgtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kcomboboxtest_SRCS
kcomboboxtest.cpp
)

kde4_automoc(${kcomboboxtest_SRCS})

kde4_add_executable(kcomboboxtest RUN_UNINSTALLED ${kcomboboxtest_SRCS})

target_link_libraries(kcomboboxtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kcompletiontest_kdeui_SRCS
kcompletionuitest.cpp
)

kde4_automoc(${kcompletiontest_kdeui_SRCS})

kde4_add_executable(kcompletiontest_kdeui RUN_UNINSTALLED ${kcompletiontest_kdeui_SRCS})

target_link_libraries(kcompletiontest_kdeui ${KDE4_KDEUI_LIBS})




set(kcompletiontest_SRCS kcompletioncoretest.cpp )

kde4_automoc(${kcompletiontest_SRCS})

kde4_add_executable(kcompletiontest RUN_UNINSTALLED ${kcompletiontest_SRCS})

target_link_libraries(kcompletiontest  ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kcompletiontest ${EXECUTABLE_OUTPUT_PATH}/kcompletiontest)

########### kmainwindow_unittest ###############

set(kmainwindow_unittest_SRCS kmainwindow_unittest.cpp )

kde4_automoc(${kmainwindow_unittest_SRCS})

kde4_add_executable(kmainwindow_unittest RUN_UNINSTALLED ${kmainwindow_unittest_SRCS})

target_link_libraries(kmainwindow_unittest  ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kmainwindow_unittest ${EXECUTABLE_OUTPUT_PATH}/kmainwindow_unittest)

########### next target ###############

set(kdatepicktest_SRCS
kdatepicktest.cpp
)

kde4_automoc(${kdatepicktest_SRCS})

kde4_add_executable(kdatepicktest RUN_UNINSTALLED ${kdatepicktest_SRCS})

target_link_libraries(kdatepicktest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kdatewidgettest_SRCS
kdatewidgettest.cpp
)

kde4_automoc(${kdatewidgettest_SRCS})

kde4_add_executable(kdatewidgettest RUN_UNINSTALLED ${kdatewidgettest_SRCS})

target_link_libraries(kdatewidgettest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(keditlistboxtest_SRCS
keditlistboxtest.cpp
)

kde4_automoc(${keditlistboxtest_SRCS})

kde4_add_executable(keditlistboxtest RUN_UNINSTALLED ${keditlistboxtest_SRCS})

target_link_libraries(keditlistboxtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kfontdialogtest_SRCS
kfontdialogtest.cpp
)

kde4_automoc(${kfontdialogtest_SRCS})

kde4_add_executable(kfontdialogtest RUN_UNINSTALLED ${kfontdialogtest_SRCS})

target_link_libraries(kfontdialogtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kgradienttest_SRCS
kgradienttest.cpp
)

kde4_automoc(${kgradienttest_SRCS})

kde4_add_executable(kgradienttest RUN_UNINSTALLED ${kgradienttest_SRCS})

target_link_libraries(kgradienttest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(khashtest_SRCS
khashtest.cpp
)

kde4_automoc(${khashtest_SRCS})

kde4_add_executable(khashtest RUN_UNINSTALLED ${khashtest_SRCS})

target_link_libraries(khashtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kdesattest_SRCS
kdesattest.cpp
)

kde4_automoc(${kdesattest_SRCS})

kde4_add_executable(kdesattest RUN_UNINSTALLED ${kdesattest_SRCS})

target_link_libraries(kdesattest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kledtest_SRCS
kledtest.cpp
)

kde4_automoc(${kledtest_SRCS})

kde4_add_executable(kledtest RUN_UNINSTALLED ${kledtest_SRCS})

target_link_libraries(kledtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(klineedittest_SRCS
klineedittest.cpp
)

kde4_automoc(${klineedittest_SRCS})

kde4_add_executable(klineedittest RUN_UNINSTALLED ${klineedittest_SRCS})

target_link_libraries(klineedittest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kmessageboxtest_SRCS
kmessageboxtest.cpp
)

kde4_automoc(${kmessageboxtest_SRCS})

kde4_add_executable(kmessageboxtest RUN_UNINSTALLED ${kmessageboxtest_SRCS})

target_link_libraries(kmessageboxtest ${KDE4_KDEUI_LIBS})

########### next target ###############

set(knuminputtest_SRCS
knuminputtest.cpp
)

kde4_automoc(${knuminputtest_SRCS})

kde4_add_executable(knuminputtest RUN_UNINSTALLED ${knuminputtest_SRCS})

target_link_libraries(knuminputtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kpalettetest_SRCS
kpalettetest.cpp
)

kde4_automoc(${kpalettetest_SRCS})

kde4_add_executable(kpalettetest RUN_UNINSTALLED ${kpalettetest_SRCS})

target_link_libraries(kpalettetest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kpopuptest_SRCS
kpopuptest.cpp
)

kde4_automoc(${kpopuptest_SRCS})

kde4_add_executable(kpopuptest RUN_UNINSTALLED ${kpopuptest_SRCS})

target_link_libraries(kpopuptest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kprogressbartest_SRCS
kprogressbartest.cpp
)

kde4_automoc(${kprogressbartest_SRCS})

kde4_add_executable(kprogressbartest RUN_UNINSTALLED ${kprogressbartest_SRCS})

target_link_libraries(kprogressbartest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kprogressdialogtest_SRCS
kprogressdialogtest.cpp
)

kde4_automoc(${kprogressdialogtest_SRCS})

kde4_add_executable(kprogressdialogtest RUN_UNINSTALLED ${kprogressdialogtest_SRCS})

target_link_libraries(kprogressdialogtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(krulertest_SRCS
krulertest.cpp
)

kde4_automoc(${krulertest_SRCS})

kde4_add_executable(krulertest RUN_UNINSTALLED ${krulertest_SRCS})

target_link_libraries(krulertest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kseparatortest_SRCS
kseparatortest.cpp
)

kde4_automoc(${kseparatortest_SRCS})

kde4_add_executable(kseparatortest RUN_UNINSTALLED ${kseparatortest_SRCS})

target_link_libraries(kseparatortest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kstatusbartest_SRCS
kstatusbartest.cpp
)

kde4_automoc(${kstatusbartest_SRCS})

kde4_add_executable(kstatusbartest RUN_UNINSTALLED ${kstatusbartest_SRCS})

target_link_libraries(kstatusbartest ${KDE4_KDEUI_LIBS})

########### next target ###############

set(ktabctltest_SRCS
ktabctltest.cpp
)

kde4_automoc(${ktabctltest_SRCS})

kde4_add_executable(ktabctltest RUN_UNINSTALLED ${ktabctltest_SRCS})

target_link_libraries(ktabctltest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kunbalancedgrdtest_SRCS
kunbalancedgrdtest.cpp
)

kde4_automoc(${kunbalancedgrdtest_SRCS})

kde4_add_executable(kunbalancedgrdtest RUN_UNINSTALLED ${kunbalancedgrdtest_SRCS})

target_link_libraries(kunbalancedgrdtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kwindowtest_SRCS
kwindowtest.cpp
)

kde4_automoc(${kwindowtest_SRCS})

kde4_add_executable(kwindowtest RUN_UNINSTALLED ${kwindowtest_SRCS})

target_link_libraries(kwindowtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(itemcontainertest_SRCS
itemcontainertest.cpp
)

kde4_automoc(${itemcontainertest_SRCS})

kde4_add_executable(itemcontainertest RUN_UNINSTALLED ${itemcontainertest_SRCS})

target_link_libraries(itemcontainertest ${KDE4_KDEUI_LIBS})

########### next target ###############

set(kxmlguitest_SRCS
kxmlguitest.cpp
)

kde4_automoc(${kxmlguitest_SRCS})

kde4_add_executable(kxmlguitest RUN_UNINSTALLED ${kxmlguitest_SRCS})

target_link_libraries(kxmlguitest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kaccelgentest_SRCS
kaccelgentest.cpp
)

kde4_automoc(${kaccelgentest_SRCS})

kde4_add_executable(kaccelgentest RUN_UNINSTALLED ${kaccelgentest_SRCS})

target_link_libraries(kaccelgentest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(ksystemtraytest_SRCS
ksystemtraytest.cpp
)

kde4_automoc(${ksystemtraytest_SRCS})

kde4_add_executable(ksystemtraytest RUN_UNINSTALLED ${ksystemtraytest_SRCS})

target_link_libraries(ksystemtraytest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(k3listviewtest_SRCS
k3listviewtest.cpp
)

kde4_automoc(${k3listviewtest_SRCS})

kde4_add_executable(k3listviewtest RUN_UNINSTALLED ${k3listviewtest_SRCS})

target_link_libraries(k3listviewtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(ktreewidgetsearchlinetest_SRCS
ktreewidgetsearchlinetest.cpp
)

kde4_automoc(${ktreewidgetsearchlinetest_SRCS})

kde4_add_executable(ktreewidgetsearchlinetest RUN_UNINSTALLED ${ktreewidgetsearchlinetest_SRCS})

target_link_libraries(ktreewidgetsearchlinetest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(ktextedittest_SRCS
ktextedittest.cpp
)

kde4_automoc(${ktextedittest_SRCS})

kde4_add_executable(ktextedittest RUN_UNINSTALLED ${ktextedittest_SRCS})

target_link_libraries(ktextedittest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kmainwindowtest_SRCS
kmainwindowtest.cpp
)

kde4_automoc(${kmainwindowtest_SRCS})

kde4_add_executable(kmainwindowtest RUN_UNINSTALLED ${kmainwindowtest_SRCS})

target_link_libraries(kmainwindowtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kmainwindowrestoretest_SRCS
kmainwindowrestoretest.cpp
)

kde4_automoc(${kmainwindowrestoretest_SRCS})

kde4_add_executable(kmainwindowrestoretest RUN_UNINSTALLED ${kmainwindowrestoretest_SRCS})

target_link_libraries(kmainwindowrestoretest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kactiontest_SRCS
kactiontest.cpp
)

kde4_automoc(${kactiontest_SRCS})

kde4_add_executable(kactiontest RUN_UNINSTALLED ${kactiontest_SRCS})

target_link_libraries(kactiontest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(ktoolbartest_SRCS
ktoolbartest.cpp
)

kde4_automoc(${ktoolbartest_SRCS})

kde4_add_executable(ktoolbartest RUN_UNINSTALLED ${ktoolbartest_SRCS})

target_link_libraries(ktoolbartest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kdatetimewidgettest_SRCS
kdatetimewidgettest.cpp
)

kde4_automoc(${kdatetimewidgettest_SRCS})

kde4_add_executable(kdatetimewidgettest RUN_UNINSTALLED ${kdatetimewidgettest_SRCS})

target_link_libraries(kdatetimewidgettest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kinputdialogtest_SRCS
kinputdialogtest.cpp
)

kde4_automoc(${kinputdialogtest_SRCS})

kde4_add_executable(kinputdialogtest RUN_UNINSTALLED ${kinputdialogtest_SRCS})

target_link_libraries(kinputdialogtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(ktabwidgettest_SRCS
ktabwidgettest.cpp
)

kde4_automoc(${ktabwidgettest_SRCS})

kde4_add_executable(ktabwidgettest RUN_UNINSTALLED ${ktabwidgettest_SRCS})

target_link_libraries(ktabwidgettest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kwordwraptest_SRCS
kwordwraptest.cpp
)

kde4_automoc(${kwordwraptest_SRCS})

kde4_add_executable(kwordwraptest RUN_UNINSTALLED ${kwordwraptest_SRCS})

target_link_libraries(kwordwraptest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(ksqueezedtextlabeltest_SRCS
ksqueezedtextlabeltest.cpp
)

kde4_automoc(${ksqueezedtextlabeltest_SRCS})

kde4_add_executable(ksqueezedtextlabeltest RUN_UNINSTALLED ${ksqueezedtextlabeltest_SRCS})

target_link_libraries(ksqueezedtextlabeltest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kpixmapregionselectordialogtest_SRCS
kpixmapregionselectordialogtest.cpp
)

kde4_automoc(${kpixmapregionselectordialogtest_SRCS})

kde4_add_executable(kpixmapregionselectordialogtest RUN_UNINSTALLED ${kpixmapregionselectordialogtest_SRCS})

target_link_libraries(kpixmapregionselectordialogtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(ktoolbarlabelactiontest_SRCS
ktoolbarlabelactiontest.cpp
)

kde4_automoc(${ktoolbarlabelactiontest_SRCS})

kde4_add_executable(ktoolbarlabelactiontest RUN_UNINSTALLED ${ktoolbarlabelactiontest_SRCS})

target_link_libraries(ktoolbarlabelactiontest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(khboxtest_SRCS
khboxtest.cpp
)

kde4_automoc(${khboxtest_SRCS})

kde4_add_executable(khboxtest RUN_UNINSTALLED ${khboxtest_SRCS})

target_link_libraries(khboxtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kbugreporttest_SRCS
kbugreporttest.cpp
)

kde4_automoc(${kbugreporttest_SRCS})

kde4_add_executable(kbugreporttest RUN_UNINSTALLED ${kbugreporttest_SRCS})

target_link_libraries(kbugreporttest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kdialogtest_SRCS
kdialogtest.cpp
)

kde4_automoc(${kdialogtest_SRCS})

kde4_add_executable(kdialogtest RUN_UNINSTALLED ${kdialogtest_SRCS})

target_link_libraries(kdialogtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kglobalacceltest_SRCS
kglobalacceltest.cpp
)

kde4_automoc(${kglobalacceltest_SRCS})

kde4_add_executable(kglobalacceltest RUN_UNINSTALLED ${kglobalacceltest_SRCS})

target_link_libraries(kglobalacceltest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(kactivelabeltest_SRCS
kactivelabeltest.cpp
)

kde4_automoc(${kactivelabeltest_SRCS})

kde4_add_executable(kactivelabeltest RUN_UNINSTALLED ${kactivelabeltest_SRCS})

target_link_libraries(kactivelabeltest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(knotificationrestrictionstest_SRCS
knotificationrestrictionstest.cpp
)

kde4_automoc(${knotificationrestrictionstest_SRCS})

kde4_add_executable(knotificationrestrictionstest RUN_UNINSTALLED ${knotificationrestrictionstest_SRCS})

target_link_libraries(knotificationrestrictionstest  ${KDE4_KDEUI_LIBS})

########### next target ###############

set(kmessagetest_SRCS
kmessagetest.cpp
)

kde4_automoc( ${kmessagetest_SRCS} )

kde4_add_executable(kmessagetest RUN_UNINSTALLED ${kmessagetest_SRCS})

target_link_libraries(kmessagetest ${KDE4_KDEUI_LIBS})

########### next target ###############
set(kpagedialogtest_SRCS
kpagedialogtest.cpp
)

kde4_automoc(${kpagedialogtest_SRCS})

kde4_add_executable(kpagedialogtest RUN_UNINSTALLED ${kpagedialogtest_SRCS})

target_link_libraries(kpagedialogtest  ${KDE4_KDEUI_LIBS})

########### next target ###############
set(kpagewidgettest_SRCS
kpagewidgettest.cpp
)

kde4_automoc(${kpagewidgettest_SRCS})

kde4_add_executable(kpagewidgettest RUN_UNINSTALLED ${kpagewidgettest_SRCS})

target_link_libraries(kpagewidgettest  ${KDE4_KDEUI_LIBS})

########### next target ###############

set(kfindtest_SRCS kfindtest.cpp )

kde4_automoc(${kfindtest_SRCS})

kde4_add_executable(kfindtest ${kfindtest_SRCS})

target_link_libraries(kfindtest ${KDE4_KDEUI_LIBS} )

########### next target ###############

set(kreplacetest_SRCS kreplacetest.cpp )

kde4_automoc(${kreplacetest_SRCS})

kde4_add_executable(kreplacetest ${kreplacetest_SRCS})

target_link_libraries(kreplacetest  ${KDE4_KDEUI_LIBS} )

########### kbuttongrouptest ###############

set(kbuttongrouptest_SRCS kbuttongrouptest.cpp )

kde4_automoc(${kbuttongrouptest_SRCS})

kde4_add_executable(kbuttongrouptest RUN_UNINSTALLED ${kbuttongrouptest_SRCS})

target_link_libraries(kbuttongrouptest  ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kbuttongrouptest ${EXECUTABLE_OUTPUT_PATH}/kbuttongrouptest)


########### kassistantdialogtest ###############

set(kassistantdialogtest_SRCS kassistantdialogtest.cpp )

kde4_automoc(${kassistantdialogtest_SRCS})

kde4_add_executable(kassistantdialogtest RUN_UNINSTALLED ${kassistantdialogtest_SRCS})

target_link_libraries(kassistantdialogtest  ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY})




endif(KDE4_BUILD_TESTS)

########### install files ###############






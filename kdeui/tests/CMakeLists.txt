set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
add_subdirectory(kconfig_compiler)

include_directories( ${KDE4_KDEUI_INCLUDES} )


MACRO(KDEUI_UNIT_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_add_unit_test(${_testname} ${_testname}.cpp)
               target_link_libraries(${_testname} ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY} ${QT_QTXML_LIBRARY} ${KDEWIN_LIBRARIES})
       ENDFOREACH(_testname)
ENDMACRO(KDEUI_UNIT_TESTS)
MACRO(KDEUI_EXECUTABLE_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_add_executable(${_testname} TEST ${_testname}.cpp)
               target_link_libraries(${_testname} ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY} ${QT_QTXML_LIBRARY} ${KDEWIN_LIBRARIES})
       ENDFOREACH(_testname)
ENDMACRO(KDEUI_EXECUTABLE_TESTS)

SET(proxyModelTestSources
  dynamictreemodel.cpp
  proxymodeltest.cpp
  modelcommander.cpp
)

MACRO(KDEUI_PROXYMODEL_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_add_unit_test(${_testname} ${_testname}.cpp ${proxyModelTestSources} )
               target_link_libraries(${_testname} ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY} ${QT_QTXML_LIBRARY} ${KDEWIN32_LIBRARIES} )
       ENDFOREACH(_testname)
ENDMACRO(KDEUI_PROXYMODEL_TESTS)


KDEUI_UNIT_TESTS(
  kactioncollectiontest
  kactioncategorytest
  kbuttongrouptest
  kcompletioncoretest
  kconfigguitest
  kconfigskeletontest
  kfadewidgeteffecttest
  kfindtest
  kglobalsettingstest
  kglobalshortcuttest
  kmainwindow_unittest
  klineedit_unittest
  ktextedit_unittest
  kcombobox_unittest
  kreplacetest
  kshortcuttest
  kstandardactiontest
  kstandardshortcuttest
  kuniqueapptest
  kwordwraptest
  kapplication_unittest
  kcolorutilstest
  kxmlgui_unittest
  ktimezonewidget_unittest
  kiconloader_unittest
  ktabwidget_unittest
  ktoolbar_unittest
)

KDEUI_PROXYMODEL_TESTS(
  descendantentitiesproxymodeltest
  selectionproxymodeltest
)

KDEUI_EXECUTABLE_TESTS(
  kaccelgentest
  kactionselectortest
  kanimatedbuttontest
  kcharselecttest
  kapptest
  kassistantdialogtest
  kbugreporttest
  kcodecactiontest
  kcolorcollectiontest
  kcolordlgtest
  kcolorcombotest
  kcomboboxtest
  kcompletionuitest
  kdatepicktest
  kdatetabletest
  kdatetimewidgettest
  kdatewidgettest
  kdebugtest_gui
  kdialogbuttonboxtest
  kdialogtest
  keditlistboxtest
  kfontdialogtest
  kglobalsettingsclient # helper program for kglobalsettingstest
  khboxtest
  kiconeffecttest
  kiconloadertest
  kinputdialogtest
  kjobtrackerstest
  kledtest
  klineedittest
  kmainwindowtest
  kmainwindowrestoretest
  kmanagerselectiontest
  kmessageboxtest
  kmessagetest
  kmodifierkeyinfotest
  knewpassworddialogtest
  knotificationrestrictionstest
  knuminputtest
  kpagedialogtest
  kpagewidgettest
  kpassivepopuptest
  kpassworddialogtest
  kpixmapregionselectordialogtest
  kpopuptest
  kprogressdialogtest
  krulertest
  kselectactiontest
  kseparatortest
  kstatusbartest
  ksqueezedtextlabeltest
  ksystemtraytest
  ktabwidgettest
  ktextedittest
  ktextbrowsertest
  ktitlewidgettest
  ktoolbartest
  ktoolbarlabelactiontest
  ktreewidgetsearchlinetest
  kwallettest
  kwidgetitemdelegatetest
  kwindowtest
  kxmlguitest
  kxmlguiwindowtest
  testqtargs
)


if (Q_WS_X11)
target_link_libraries(kmanagerselectiontest ${X11_X11_LIB})

KDEUI_EXECUTABLE_TESTS(
 fixx11h_test
 fixx11h_test2
 kxerrorhandlertest
)
target_link_libraries(kxerrorhandlertest ${X11_X11_LIB})
endif (Q_WS_X11)

## kplottest

set(kplottest_SRCS testplot_widget.cpp testplot_main.cpp)
kde4_add_executable(kplottest TEST ${kplottest_SRCS})
target_link_libraries(kplottest ${KDE4_KDEUI_LIBS})

add_subdirectory(proxymodeltestapp)


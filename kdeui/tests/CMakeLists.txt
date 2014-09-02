set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
add_subdirectory(kconfig_compiler)

include_directories( ${KDE4_KDEUI_INCLUDES} proxymodeltestsuite )

# QtTest needs this in order to include the gui stuff.
# The alternative (which requires Qt>=4.6) is #include <QTestGui>
add_definitions(-DQT_GUI_LIB)

option(MAC_USE_OSXKEYCHAIN "On OS X, use the keychain as backend for kwallet, instead of kwalletd.")

MACRO(KDEUI_UNIT_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_add_unit_test(${_testname} TESTNAME kdeui-${_testname} ${_testname}.cpp)
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
  kselectionproxymodeltestsuite.cpp
)

add_subdirectory(proxymodeltestsuite)

MACRO(KDEUI_PROXYMODEL_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_add_unit_test(${_testname} TESTNAME kdeui-${_testname} ${_testname}.cpp ${proxyModelTestSources} )
               target_link_libraries(${_testname} ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY} ${QT_QTXML_LIBRARY} ${KDEWIN32_LIBRARIES} proxymodeltestsuite )
       ENDFOREACH(_testname)
ENDMACRO(KDEUI_PROXYMODEL_TESTS)


KDEUI_UNIT_TESTS(
  kactioncollectiontest
  kactioncategorytest
  kbuttongrouptest
  kcompletioncoretest
  kconfigguitest
  kconfigskeletontest
  kdualactiontest
  kfadewidgeteffecttest
  kfindtest
  kglobalsettingstest
  kglobalshortcuttest
  kmainwindow_unittest
  klineedit_unittest
  ktextedit_unittest
  kcombobox_unittest
  kdialog_unittest
  kreplacetest
  kshortcuttest
  kstandardactiontest
  kstandardshortcuttest
  kuniqueapptest
  kwordwraptest
  kapplication_unittest
  kstartupinfo_unittest
  kcolorutilstest
  kxmlgui_unittest
  ktimezonewidget_unittest
  kiconloader_unittest
  ktabwidget_unittest
  ktoolbar_unittest
  krichtextedittest
  kselectaction_unittest
  klistwidgetsearchlinetest
  ktimecomboboxtest
  kdatecomboboxtest
  kdatetimeedittest
  kconfigdialog_unittest
  kcolorbuttontest
  klinkitemselectionmodeltest
)

KDEUI_PROXYMODEL_TESTS(
  kdescendantsproxymodeltest
  kselectionproxymodeltest
  testmodelqueuedconnections
)

KDEUI_EXECUTABLE_TESTS(
  kaccelgentest
  kactionselectortest
  kanimatedbuttontest
  kcharselecttest
  kapptest
  kassistantdialogtest
  kbugreporttest
  kcategorizedviewtest
  kcodecactiontest
  kcolorcollectiontest
  kcolordlgtest
  kcolorcombotest
  kcomboboxtest
  kcompletionuitest
  kdatepicktest
  klanguagebuttontest
  kdatetabletest
  kdatetimewidgettest
  kdatewidgettest
  kdebugtest_gui
  kdialogbuttonboxtest
  kdialogtest
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
  kmessageboxtest
  kmessagetest
  kmessagewidgettest
  kmodifierkeyinfotest
  knewpassworddialogtest
  kstatusnotifieritemtest
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
  kpixmapsequenceoverlaypaintertest
)

if (Q_WS_MAC AND MAC_USE_OSXKEYCHAIN)
  set_source_files_properties(kwallettest.cpp PROPERTIES 
                               COMPILE_FLAGS -DMAC_USE_OSXKEYCHAIN)
endif(Q_WS_MAC AND MAC_USE_OSXKEYCHAIN)

if (NOT KDE_NO_DEPRECATED)
KDEUI_EXECUTABLE_TESTS(
  keditlistboxtest
)
endif (NOT KDE_NO_DEPRECATED)

if (Q_WS_X11)

KDEUI_EXECUTABLE_TESTS(
 fixx11h_test
 fixx11h_test2
 kxerrorhandlertest
 kmanagerselectiontest
)
target_link_libraries(kmanagerselectiontest ${X11_X11_LIB})
target_link_libraries(kxerrorhandlertest ${X11_X11_LIB})
endif (Q_WS_X11)

## kplottest

set(kplottest_SRCS testplot_widget.cpp testplot_main.cpp)
kde4_add_executable(kplottest TEST ${kplottest_SRCS})
target_link_libraries(kplottest ${KDE4_KDEUI_LIBS})

## kcolorutilsdemo

SET(kcolorUtilsDemoSources kcolorutilsdemo.cpp kimageframe.cpp ../colors/kcolorspaces.cpp)
kde4_add_ui_files(kcolorUtilsDemoSources kcolorutilsdemo.ui)
kde4_add_executable(kcolorutilsdemo ${kcolorUtilsDemoSources})
target_link_libraries(kcolorutilsdemo ${KDE4_KDEUI_LIBS} ${KDEWIN_LIBRARIES})


add_subdirectory(proxymodeltestapp)


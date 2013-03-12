set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${KDE4_KDEUI_INCLUDES} proxymodeltestsuite )

include(ECMMarkAsTest)

MACRO(KDEUI_UNIT_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_add_unit_test(${_testname} TESTNAME kdeui-${_testname} ${_testname}.cpp)
               target_link_libraries(${_testname} ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY} ${QT_QTXML_LIBRARY} ${KDEWIN_LIBRARIES} ${KCoreAddons_LIBRARIES} ${KWidgetsAddons_LIBRARIES})
       ENDFOREACH(_testname)
ENDMACRO(KDEUI_UNIT_TESTS)
MACRO(KDEUI_EXECUTABLE_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_add_executable(${_testname} TEST ${_testname}.cpp)
               target_link_libraries(${_testname} ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY} ${QT_QTXML_LIBRARY} ${KDEWIN_LIBRARIES} ${KCoreAddons_LIBRARIES} ${KWidgetsAddons_LIBRARIES})
       ENDFOREACH(_testname)
ENDMACRO(KDEUI_EXECUTABLE_TESTS)

KDEUI_UNIT_TESTS(
  kactioncollectiontest
  kactioncategorytest
  kbuttongrouptest
  kcompletioncoretest
  kdualactiontest
  kfindtest
  kglobalshortcuttest
  kmainwindow_unittest
  klineedit_unittest
  ktextedit_unittest
  kcombobox_unittest
  kreplacetest
  kshortcuttest
  kstandardactiontest
  kstandardshortcuttest
  kxmlgui_unittest
  ktimezonewidget_unittest
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
)

KDEUI_EXECUTABLE_TESTS(
  kaccelgentest
  kactionselectortest
  kanimatedbuttontest
  kcharselecttest
  kassistantdialogtest
  kbugreporttest
  kcategorizedviewtest
  kcodecactiontest
  kcolorcollectiontest
  kcolorcombotest
  kcomboboxtest
  kcompletionuitest
  kdatepicktest
  kdatetabletest
  kdatetimewidgettest
  kdatewidgettest
  kdebugtest_gui
  keditlistwidgettest
  kfontdialogtest
  kicondialogtest
  kinputdialogtest
  kjobtrackerstest
  klineedittest
  kmainwindowtest
  kmainwindowrestoretest
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
  krulertest
  kselectactiontest
  ksqueezedtextlabeltest
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

if (NOT KDE_NO_DEPRECATED)
KDEUI_EXECUTABLE_TESTS(
  keditlistboxtest
)
endif (NOT KDE_NO_DEPRECATED)

if (X11_FOUND)
find_package(Qt5Transitional REQUIRED MODULE COMPONENTS X11Extras)
include_directories(${Qt5X11Extras_INCLUDE_DIRS})
KDEUI_EXECUTABLE_TESTS(
 kxerrorhandlertest
)

# TODO! PORT TO NEW KManagerSelection API!
#KDEUI_UNIT_TESTS(
#kmanagerselectiontest
#)
#target_link_libraries(kmanagerselectiontest ${X11_X11_LIB})
target_link_libraries(kxerrorhandlertest ${X11_X11_LIB})
endif (X11_FOUND)

## kcolorutilsdemo

SET(kcolorUtilsDemoSources kcolorutilsdemo.cpp kimageframe.cpp)
kde4_add_ui_files(kcolorUtilsDemoSources kcolorutilsdemo.ui)
add_executable(kcolorutilsdemo ${kcolorUtilsDemoSources})
ecm_mark_as_test(kcolorutilsdemo)
target_link_libraries(kcolorutilsdemo ${KDE4_KDEUI_LIBS} ${KDEWIN_LIBRARIES} ${KCoreAddons_LIBRARIES} )

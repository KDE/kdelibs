set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include(ECMMarkAsTest)

MACRO(KDEUI_UNIT_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_add_unit_test(${_testname} TESTNAME kdeui-${_testname} ${_testname}.cpp)
               target_link_libraries(${_testname} ${KDE4_KDEUI_LIBS} Qt5::Test Qt5::Xml kde4support  ${KDEWIN_LIBRARIES} ${KCoreAddons_LIBRARIES} ${KWidgetsAddons_LIBRARIES})
       ENDFOREACH(_testname)
ENDMACRO(KDEUI_UNIT_TESTS)
MACRO(KDEUI_EXECUTABLE_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_add_executable(${_testname} TEST ${_testname}.cpp)
               target_link_libraries(${_testname} ${KDE4_KDEUI_LIBS} Qt5::Test Qt5::Xml ${KDEWIN_LIBRARIES} ${KCoreAddons_LIBRARIES} ${KWidgetsAddons_LIBRARIES})
       ENDFOREACH(_testname)
ENDMACRO(KDEUI_EXECUTABLE_TESTS)

KDEUI_UNIT_TESTS(
  kactioncollectiontest
  kactioncategorytest
  kglobalsettingstest
  kmainwindow_unittest
  kxmlgui_unittest
  ktoolbar_unittest
  krichtextedittest
  kconfigdialog_unittest
  kcolorbuttontest
  klinkitemselectionmodeltest
)

KDEUI_EXECUTABLE_TESTS(
  kbugreporttest
  kcategorizedviewtest
  kcolorcollectiontest
  kdebugtest_gui
  kglobalsettingsclient # helper program for kglobalsettingstest
  kmainwindowtest
  kmainwindowrestoretest
  knewpassworddialogtest
  knotificationrestrictionstest
  kpopuptest
  krulertest
  ktoolbartest
  kwindowtest
  kxmlguitest
  kxmlguiwindowtest
)

if (X11_FOUND)
find_package(Qt5X11Extras 5.2.0 REQUIRED NO_MODULE)

# TODO! PORT TO NEW KManagerSelection API!
#KDEUI_UNIT_TESTS(
#kmanagerselectiontest
#)
#target_link_libraries(kmanagerselectiontest ${X11_X11_LIB})
endif (X11_FOUND)

## kcolorutilsdemo

SET(kcolorUtilsDemoSources kcolorutilsdemo.cpp kimageframe.cpp)
kde4_add_ui_files(kcolorUtilsDemoSources kcolorutilsdemo.ui)
add_executable(kcolorutilsdemo ${kcolorUtilsDemoSources})
ecm_mark_as_test(kcolorutilsdemo)
target_link_libraries(kcolorutilsdemo ${KDE4_KDEUI_LIBS} ${KDEWIN_LIBRARIES} ${KCoreAddons_LIBRARIES} )

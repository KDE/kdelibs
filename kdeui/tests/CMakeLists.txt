set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
add_subdirectory(kconfig_compiler)

include_directories( ${KDE4_KDEUI_INCLUDES} )


MACRO(KDEUI_UNIT_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_add_unit_test(${_testname} ${_testname}.cpp)
               target_link_libraries(${_testname} ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY})
       ENDFOREACH(_testname)
ENDMACRO(KDEUI_UNIT_TESTS)
MACRO(KDEUI_EXECUTABLE_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_add_executable(${_testname} TEST ${_testname}.cpp)
               target_link_libraries(${_testname} ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY})
       ENDFOREACH(_testname)
ENDMACRO(KDEUI_EXECUTABLE_TESTS)

KDEUI_UNIT_TESTS(
  kactioncollectiontest
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
)

KDEUI_EXECUTABLE_TESTS(
  kpassivepopuptest
  kdialogbuttonboxtest
  kcharselecttest
  kcolordlgtest
  kcolorcombotest
  kcomboboxtest
  kcompletionuitest
  kdatepicktest
  kdatewidgettest
  keditlistboxtest
  kfontdialogtest
  kledtest
  klineedittest
  kmessageboxtest
  knuminputtest
  kcolorcollectiontest
  kpopuptest
  kprogressdialogtest
  krulertest
  kseparatortest
  kstatusbartest
  kwindowtest
  kselectactiontest
  kxmlguitest
  kaccelgentest
  ksystemtraytest
  ktreewidgetsearchlinetest
  ktextedittest
  kmainwindowtest
  kmainwindowrestoretest
  kcodecactiontest
  ktoolbartest
  kdatetimewidgettest
  kdatetabletest
  kinputdialogtest
  ktabwidgettest
  ksqueezedtextlabeltest
  kpixmapregionselectordialogtest
  ktoolbarlabelactiontest
  khboxtest
  kbugreporttest
  kdialogtest
  knotificationrestrictionstest
  kmessagetest
  kpagedialogtest
  kpagewidgettest
  kassistantdialogtest
  kactionselectortest
  kapptest
  testqtargs
  kiconloadertest
  kglobalsettingsclient # helper program for kglobalsettingstest
  knewpassworddialogtest
  kpassworddialogtest
  kjobtrackerstest
  kdebugtest_gui
  ktitlewidgettest
  kiconeffecttest
  kwallettest
  kwidgetitemdelegatetest
)

if (Q_WS_X11)
KDEUI_EXECUTABLE_TESTS(
 kxerrorhandlertest
)
endif (Q_WS_X11)

## kplottest

set(kplottest_SRCS testplot_widget.cpp testplot_main.cpp)
kde4_add_executable(kplottest TEST ${kplottest_SRCS})
target_link_libraries(kplottest ${KDE4_KDEUI_LIBS})

add_subdirectory(kconfig_compiler)

# This can be used for finding .rc files in the source dir, without installing them
add_definitions( -DKDESRCDIR=\\"${CMAKE_CURRENT_SOURCE_DIR}\\" )

include_directories( ${KDE4_KDEUI_INCLUDES} )


MACRO(KDEUI_UNIT_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_automoc(${_testname}.cpp)
               kde4_add_unit_test(${_testname} ${_testname}.cpp)
               target_link_libraries(${_testname} ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY})
       ENDFOREACH(_testname)
ENDMACRO(KDEUI_UNIT_TESTS)
MACRO(KDEUI_EXECUTABLE_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_automoc(${_testname}.cpp)
               kde4_add_test_executable(${_testname} ${_testname}.cpp)
               target_link_libraries(${_testname} ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY})
       ENDFOREACH(_testname)
ENDMACRO(KDEUI_EXECUTABLE_TESTS)

KDEUI_UNIT_TESTS(
  kactioncollectiontest
  kbuttongrouptest
  kcompletioncoretest
  kconfigguitest
  kconfigskeletontest
  kfindtest
  kglobalsettingstest
  kmainwindow_unittest
  kreplacetest
  kshortcuttest
  kstandardactiontest
  kstandardshortcuttest
  kuniqueapptest
  kapplication_unittest
  kcolorutilstest
)

KDEUI_EXECUTABLE_TESTS(
  kpassivepopuptest
  kblendtest
  kdialogbuttonboxtest
  kcharselecttest
  kcolortest
  kcolordlgtest
  kcolorcombotest
  kcomboboxtest
  kcompletionuitest
  kdatepicktest
  kdatewidgettest
  keditlistboxtest
  kfontdialogtest
  kgradienttest
  khashtest
  kdesattest
  kledtest
  klineedittest
  kmessageboxtest
  knuminputtest
  kpalettetest
  kpopuptest
  kprogressdialogtest
  krulertest
  kseparatortest
  kstatusbartest
  kunbalancedgrdtest
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
  kwordwraptest
  ksqueezedtextlabeltest
  kpixmapregionselectordialogtest
  ktoolbarlabelactiontest
  khboxtest
  kbugreporttest
  kdialogtest
  kglobalacceltest
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
)

if (Q_WS_X11)
KDEUI_EXECUTABLE_TESTS(
 kxerrorhandlertest
)
endif (Q_WS_X11)

## kplottest

set(kplottest_SRCS testplot_widget.cpp testplot_main.cpp)
kde4_automoc(${kplottest_SRCS})
kde4_add_test_executable(kplottest ${kplottest_SRCS})
target_link_libraries(kplottest ${KDE4_KDEUI_LIBS})

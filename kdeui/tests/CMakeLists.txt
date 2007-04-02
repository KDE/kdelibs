add_subdirectory(kconfig_compiler)

# Tests don't need to go into toplevel/bin, they are fine in the current dir.
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

# This can be used for finding .rc files in the source dir, without installing them
add_definitions( -DKDESRCDIR=\\"${CMAKE_CURRENT_SOURCE_DIR}\\" )

include_directories( ${KDE4_KDEUI_INCLUDES} )


MACRO(KDEUI_BUILD_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_automoc(${_testname}.cpp)
               kde4_add_test(${_testname} ${_testname}.cpp)
               target_link_libraries(${_testname} ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY})
       ENDFOREACH(_testname)
ENDMACRO(KDEUI_BUILD_TESTS)
MACRO(KDEUI_EXECUTE_TESTS)
       FOREACH(_testname ${ARGN})
               add_test(kdeui-${_testname} ${EXECUTABLE_OUTPUT_PATH}/${_testname})
       ENDFOREACH(_testname)
ENDMACRO(KDEUI_EXECUTE_TESTS)

set(kdeui_unittests
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
)

KDEUI_BUILD_TESTS(
  ${kdeui_unittests}
  kpassivepopuptest
  kblendtest
  kdialogbuttonboxtest
  kcharselecttest
  kcolortest
  kcolordlgtest
  kcomboboxtest
  kcompletionuitest
  kdatepicktest
  kdatewidgettest
  keditlistboxtest
  kfontdialogtest
  kgradienttest
  khashtest
  kdesattest
  kinformationlabeltest
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
)

if (Q_WS_X11)
KDEUI_BUILD_TESTS(
 kxerrorhandlertest
)
endif (Q_WS_X11)

KDEUI_EXECUTE_TESTS(
  ${kdeui_unittests}
)

## kplottest

set(kplottest_SRCS testplot_widget.cpp testplot_main.cpp)
kde4_automoc(${kplottest_SRCS})
kde4_add_test(kplottest ${kplottest_SRCS})
target_link_libraries(kplottest ${KDE4_KDEUI_LIBS})

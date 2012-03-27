set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${KDE4_KDEUI_INCLUDES} proxymodeltestsuite )

# QtTest needs this in order to include the gui stuff.
# The alternative (which requires Qt>=4.6) is #include <QTestGui>
add_definitions(-DQT_GUI_LIB)

add_definitions(-DQT_WIDGETS_LIB)


MACRO(KDEUI_UNIT_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_add_unit_test(${_testname} TESTNAME kdeui-${_testname} ${_testname}.cpp)
               target_link_libraries(${_testname} ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY} ${QT_QTXML_LIBRARY} ${KDEWIN_LIBRARIES} kcoreaddons)
       ENDFOREACH(_testname)
ENDMACRO(KDEUI_UNIT_TESTS)
MACRO(KDEUI_EXECUTABLE_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_add_executable(${_testname} TEST ${_testname}.cpp)
               target_link_libraries(${_testname} ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY} ${QT_QTXML_LIBRARY} ${KDEWIN_LIBRARIES} kcoreaddons)
       ENDFOREACH(_testname)
ENDMACRO(KDEUI_EXECUTABLE_TESTS)

KDEUI_EXECUTABLE_TESTS(
  khboxtest
  kdialogtest
  kiconloadertest
  kiconeffecttest
  kseparatortest
  kglobalsettingsclient # helper program for kglobalsettingstest
)

KDEUI_UNIT_TESTS(
  kglobalsettingstest
  kdialog_unittest
  kiconloader_unittest
)

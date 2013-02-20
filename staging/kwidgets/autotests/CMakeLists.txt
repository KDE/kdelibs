include_directories( ${KDE4_KDEUI_INCLUDES} )

# QtTest needs this in order to include the gui stuff.
# The alternative (which requires Qt>=4.6) is #include <qtest_gui.h>
# TODO: include <QTestWidgets> if https://codereview.qt-project.org/26239 goes in
add_definitions(-DQT_GUI_LIB)

add_definitions(-DQT_WIDGETS_LIB)

include(ECMMarkAsTest)

macro(kwidgets_unit_tests)
   foreach(_testname ${ARGN})
      add_executable(${_testname} ${_testname}.cpp)
      add_test(kwidgets-${_testname} ${_testname})
      target_link_libraries(${_testname} ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTMAIN_LIBRARY} ${KDEWIN_LIBRARIES} kcoreaddons)
      ecm_mark_as_test(${_testname})
   endforeach()
endmacro()

macro(kwidgets_executable_tests)
   foreach(_testname ${ARGN})
      add_executable(${_testname} ${_testname}.cpp)
      target_link_libraries(${_testname} ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTMAIN_LIBRARY} ${KDEWIN_LIBRARIES} kcoreaddons)
      ecm_mark_as_test(${_testname})
   endforeach()
endmacro()

kwidgets_executable_tests(
    kglobalsettingsclient # helper program for kglobalsettingstest
)

kwidgets_unit_tests(
    kglobalsettingstest
    kiconloader_unittest
)

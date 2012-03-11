
set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${KDE4_KDEUI_INCLUDES} )

# QtTest needs this in order to include the gui stuff.
# The alternative (which requires Qt>=4.6) is #include <QTestGui>
add_definitions(-DQT_GUI_LIB)

add_definitions(-DQT_WIDGETS_LIB)


macro(kwidgets_unit_tests)
       foreach(_testname ${ARGN})
               kde4_add_unit_test(${_testname} TESTNAME kdeui-${_testname} ${_testname}.cpp)
               target_link_libraries(${_testname} ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY} ${QT_QTXML_LIBRARY} ${KDEWIN_LIBRARIES} kcoreaddons)
       endforeach()
endmacro()
macro(kwidgets_executable_tests)
       foreach(_testname ${ARGN})
               kde4_add_executable(${_testname} TEST ${_testname}.cpp)
               target_link_libraries(${_testname} ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY} ${QT_QTXML_LIBRARY} ${KDEWIN_LIBRARIES} kcoreaddons)
       endforeach()
endmacro()

kwidgets_executable_tests(
    kglobalsettingsclient # helper program for kglobalsettingstest
    kiconloadertest
    kiconeffecttest
    khboxtest
    kseparatortest
    kdialogtest
)

kwidgets_unit_tests(
    kglobalsettingstest
    kiconloader_unittest
    kdialog_unittest
)

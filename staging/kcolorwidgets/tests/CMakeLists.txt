set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(
 ${KDE4_KDECORE_INCLUDES}
 ${DBUSMENUQT_INCLUDE_DIR}
 ${CMAKE_CURRENT_SOURCE_DIR}/../src
 ${CMAKE_CURRENT_BINARY_DIR}/../src
 ${kwidgets_BINARY_DIR}/src
 ${kwidgets_SOURCE_DIR}/src/dialogs # kdialog
 ${kwidgets_SOURCE_DIR}/src/utils
 ${kwidgets_SOURCE_DIR}/src/icons
 ${kwidgets_SOURCE_DIR}/src/widgets
)


add_definitions(-DQT_GUI_LIB)

add_definitions(-DQT_WIDGETS_LIB)

MACRO(KDEUI_EXECUTABLE_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_add_executable(${_testname} TEST ${_testname}.cpp)
               target_link_libraries(${_testname} ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY} ${QT_QTXML_LIBRARY} ${KDEWIN_LIBRARIES} kcoreaddons)
       ENDFOREACH(_testname)
ENDMACRO(KDEUI_EXECUTABLE_TESTS)

KDEUI_EXECUTABLE_TESTS(
  kcolorcombotest
)

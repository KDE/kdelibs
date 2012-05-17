set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${KDE4_KDEUI_INCLUDES} )

macro(kwidgets_executable_tests)
       foreach(_testname ${ARGN})
               add_executable(${_testname} ${_testname}.cpp)
               target_link_libraries(${_testname} ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY} ${QT_QTXML_LIBRARY} ${KDEWIN_LIBRARIES} kcoreaddons)
       endforeach()
endmacro()

kwidgets_executable_tests(
    kiconloadertest
    kiconeffecttest
    kdialogtest
)


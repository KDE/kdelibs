set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
include_directories(${KDE4_KDECORE_INCLUDES})

macro(KUNITCONVERSION_UNIT_TESTS)
    foreach(_testname ${ARGN})
        kde4_add_unit_test(${_testname} ${_testname}.cpp)
        target_link_libraries(${_testname} ${KDE4_KDECORE_LIBS} kunitconversion ${QT_QTTEST_LIBRARY} ${QT_QTGUI_LIBRARY} )
    endforeach(_testname)
endmacro(KUNITCONVERSION_UNIT_TESTS)

KUNITCONVERSION_UNIT_TESTS(
    valuetest
    categorytest
    convertertest
)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

include_directories("${CMAKE_CURRENT_BINARY_DIR}/../..")

macro(KUNITCONVERSION_UNIT_TESTS)
    foreach(_testname ${ARGN})
        kde4_add_unit_test(${_testname} TESTNAME kunitconversion-${_testname} ${_testname}.cpp)
        target_link_libraries(${_testname} ${KDE4_KDECORE_LIBS} kunitconversion ${QT_QTTEST_LIBRARY}
                              Qt5::Widgets ${KCoreAddons_LIBRARIES} )
    endforeach(_testname)
endmacro(KUNITCONVERSION_UNIT_TESTS)

KUNITCONVERSION_UNIT_TESTS(
    valuetest
    categorytest
    convertertest
)

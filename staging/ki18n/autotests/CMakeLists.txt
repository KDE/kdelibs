remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include(ECMMarkAsTest)

macro(KI18N_UNIT_TESTS)
    foreach(_testname ${ARGN})
        add_executable(${_testname} ${_testname}.cpp)
        target_link_libraries(${_testname} ${QT_QTTEST_LIBRARY} ki18n)
        add_test(ki18n-${_testname} ${_testname})
        ecm_mark_as_test(${_testname})
    endforeach()
endmacro()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src ${CMAKE_CURRENT_BINARY_DIR}/../src)

ki18n_unit_tests(
    klocalizedstringtest
)


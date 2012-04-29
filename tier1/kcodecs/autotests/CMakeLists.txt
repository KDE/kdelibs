set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

macro(KCODECS_EXECUTABLE_TESTS)
    foreach(_testname ${ARGN})
        add_executable(${_testname} ${_testname}.cpp)
        target_link_libraries(${_testname} ${QT_QTCORE_LIBRARY} kcodecs)
    endforeach()
endmacro()
macro(KCODECS_UNIT_TESTS)
    foreach(_testname ${ARGN})
        KCODECS_EXECUTABLE_TESTS(${_testname})
        target_link_libraries(${_testname} ${QT_QTTEST_LIBRARY})
        add_test(kcodecs-${_testname} ${_testname})
    endforeach()
endmacro()

kcodecs_unit_tests(
    kcharsetstest
    kencodingdetectortest
    kmacroexpandertest
    kstringhandlertest
)

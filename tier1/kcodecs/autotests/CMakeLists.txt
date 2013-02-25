remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include(ECMMarkAsTest)

macro(KCODECS_EXECUTABLE_TESTS)
    foreach(_testname ${ARGN})
        add_executable(${_testname} ${_testname}.cpp)
        target_link_libraries(${_testname} KCodecs)
    endforeach()
endmacro()

macro(KCODECS_UNIT_TESTS)
    foreach(_testname ${ARGN})
        kcodecs_executable_tests(${_testname})
        target_link_libraries(${_testname} ${QT_QTTEST_LIBRARY} ${QT_QTMAIN_LIBRARY})
        add_test(kcodecs-${_testname} ${_testname})
        ecm_mark_as_test(${_testname})
    endforeach()
endmacro()

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/../src
   ${CMAKE_CURRENT_BINARY_DIR}/../src
)

kcodecs_unit_tests(
    kcharsetstest
    kencodingprobertest
)

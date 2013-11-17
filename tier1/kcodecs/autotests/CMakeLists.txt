remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include(ECMMarkAsTest)

find_package(Qt5Test 5.2.0 REQUIRED NO_MODULE)

macro(KCODECS_EXECUTABLE_TESTS)
    foreach(_testname ${ARGN})
        add_executable(${_testname} ${_testname}.cpp)
        target_link_libraries(${_testname} KCodecs)
    endforeach()
endmacro()

macro(KCODECS_UNIT_TESTS)
    foreach(_testname ${ARGN})
        kcodecs_executable_tests(${_testname})
        target_link_libraries(${_testname} Qt5::Test)
        add_test(kcodecs-${_testname} ${_testname})
        ecm_mark_as_test(${_testname})
    endforeach()
endmacro()

kcodecs_unit_tests(
    kcharsetstest
    kencodingprobertest
)

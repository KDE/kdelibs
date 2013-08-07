remove_definitions(-DQT_NO_CAST_FROM_ASCII)

macro(KDBUSADDONS_TESTS)
    foreach(_testname ${ARGN})
        add_executable(${_testname} ${_testname}.cpp)
        target_link_libraries(${_testname} KDBusAddons)
    endforeach()
endmacro()

kdbusaddons_tests(
   dbuscalltest
)


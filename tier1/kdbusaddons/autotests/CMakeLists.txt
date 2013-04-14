
remove_definitions(-DQT_NO_CAST_FROM_ASCII)

find_package(Qt5Transitional REQUIRED MODULE COMPONENTS Test)

include(ECMMarkAsTest)

macro(KDBUSADDONS_UNIT_TESTS)
    foreach(_testname ${ARGN})
        add_executable(${_testname} ${_testname}.cpp)
        add_test(${_testname} ${_testname})
        target_link_libraries(${_testname} Qt5::DBus ${QT_QTTEST_LIBRARY} KDBusAddons)
        ecm_mark_as_test(${_testname})
    endforeach()
endmacro()


########### next target ###############

kdbusaddons_unit_tests(
    kdbusinterprocesslocktest
    kdbusservicetest
)


remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include(ECMMarkAsTest)

find_package(Qt5 5.2.0 CONFIG REQUIRED Concurrent Network)

macro(KCOREADDONS_EXECUTABLE_TESTS)
    foreach(_testname ${ARGN})
        add_executable(${_testname} ${_testname}.cpp)
        # TODO: split out the QtNetwork-dependent stuff into a separate addon?
        target_link_libraries(${_testname} Qt5::Network KCoreAddons )
        ecm_mark_as_test(${_testname})
    endforeach()
endmacro()

macro(KCOREADDONS_UNIT_TESTS)
    foreach(_testname ${ARGN})
        kcoreaddons_executable_tests(${_testname})
        target_link_libraries(${_testname} Qt5::Test Qt5::Concurrent)
        add_test(kcoreaddons-${_testname} ${_testname})
        ecm_mark_as_test(${_testname})
    endforeach()
endmacro()

kcoreaddons_unit_tests(
    kaboutdatatest
    kautosavefiletest
    kcompositejobtest
    kdirwatch_unittest
    kjobtest
    kprocesstest
    kshareddatacachetest
    kshelltest
    kurlmimedatatest
    kstringhandlertest
)


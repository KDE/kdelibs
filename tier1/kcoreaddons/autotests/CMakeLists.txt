remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include(ECMMarkAsTest)

find_package(Qt5 REQUIRED NO_MODULE COMPONENTS Concurrent Network)

macro(KCOREADDONS_EXECUTABLE_TESTS)
    foreach(_testname ${ARGN})
        add_executable(${_testname} ${_testname}.cpp)
        # TODO: split out the QtNetwork-dependent stuff into a separate addon?
        target_link_libraries(${_testname} ${QT_QTNETWORK_LIBRARY} KCoreAddons )
        ecm_mark_as_test(${_testname})
    endforeach()
endmacro()

macro(KCOREADDONS_UNIT_TESTS)
    foreach(_testname ${ARGN})
        kcoreaddons_executable_tests(${_testname})
        target_link_libraries(${_testname} ${QT_QTTEST_LIBRARY} Qt5::Concurrent)
        add_test(kcoreaddons-${_testname} ${_testname})
        ecm_mark_as_test(${_testname})
    endforeach()
endmacro()

kcoreaddons_unit_tests(
    kaboutdatatest
    kjobtest
    klockfiletest
    kautosavefiletest
    kurlmimedatatest
    kdirwatch_unittest
    kshareddatacachetest
)

# Helper for klockfiletest
kcoreaddons_executable_tests(
    klockfile_testlock
)


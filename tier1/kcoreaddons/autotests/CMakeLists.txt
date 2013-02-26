remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include(ECMMarkAsTest)

macro(KCOREADDONS_EXECUTABLE_TESTS)
    foreach(_testname ${ARGN})
        add_executable(${_testname} ${_testname}.cpp)
        # TODO: split out the QtNetwork-dependent stuff into a separate addon?
        target_link_libraries(${_testname} ${QT_QTNETWORK_LIBRARY} ${QT_QTMAIN_LIBRARY} KCoreAddons )
        ecm_mark_as_test(${_testname})
    endforeach()
endmacro()

macro(KCOREADDONS_UNIT_TESTS)
    foreach(_testname ${ARGN})
        kcoreaddons_executable_tests(${_testname})
        target_link_libraries(${_testname} ${QT_QTTEST_LIBRARY} ${QT_QTMAIN_LIBRARY})
        add_test(kcoreaddons-${_testname} ${_testname})
        ecm_mark_as_test(${_testname})
    endforeach()
endmacro()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/kernel
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/jobs
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/io
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/caching
    ${CMAKE_CURRENT_BINARY_DIR}/../src
)

kcoreaddons_unit_tests(
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


set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

MACRO(KCOREADDONS_EXECUTABLE_TESTS)
       FOREACH(_testname ${ARGN})
               add_executable(${_testname} ${_testname}.cpp)
               # TODO: split out the QtNetwork-dependent stuff into a separate addon?
               target_link_libraries(${_testname} ${QT_QTNETWORK_LIBRARY} ${QT_QTCORE_LIBRARY} kcoreaddons)
               if(WINCE)
                target_link_libraries(${_testname} ${WCECOMPAT_LIBRARIES})
               endif(WINCE)
       ENDFOREACH(_testname)
ENDMACRO(KCOREADDONS_EXECUTABLE_TESTS)
MACRO(KCOREADDONS_UNIT_TESTS)
       FOREACH(_testname ${ARGN})
               KCOREADDONS_EXECUTABLE_TESTS(${_testname})
               target_link_libraries(${_testname} ${QT_QTTEST_LIBRARY})
               add_test(kcoreaddons-${_testname} ${_testname})
       ENDFOREACH(_testname)
ENDMACRO(KCOREADDONS_UNIT_TESTS)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/kernel
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/text
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/jobs
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/io
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/caching
    ${CMAKE_CURRENT_BINARY_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../.. # kde_qt5_compat.h
)

KCOREADDONS_UNIT_TESTS(
    kaboutdatatest
    kcharsetstest
    kjobtest
    klockfiletest
    kautosavefiletest
    kurlmimedatatest
    kencodingdetectortest
    kdirwatch_unittest
    kmacroexpandertest
    kstringhandlertest
    kshareddatacachetest
)

# Helper for klockfiletest
KCOREADDONS_EXECUTABLE_TESTS(
    klockfile_testlock
)


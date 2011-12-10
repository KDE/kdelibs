MACRO(KCOREADDONS_UNIT_TESTS)
       FOREACH(_testname ${ARGN})
               #kde4_add_unit_test(${_testname} TESTNAME "kcoreaddons-${_testname}" NOGUI ${_testname}.cpp)
	       add_executable(${_testname} ${_testname}.cpp)
	       add_test(kcoreaddons-${_testname} ${CMAKE_BINARY_DIR}/bin/${_testname})
               target_link_libraries(${_testname} ${QT_QTTEST_LIBRARY} ${QT_QTNETWORK_LIBRARY} kcoreaddons)
               if(WINCE)
                target_link_libraries(${_testname} ${WCECOMPAT_LIBRARIES})
               endif(WINCE)
       ENDFOREACH(_testname)
ENDMACRO(KCOREADDONS_UNIT_TESTS)
MACRO(KCOREADDONS_EXECUTABLE_TESTS)
       FOREACH(_testname ${ARGN})
	       add_executable(${_testname} ${_testname}.cpp)
               target_link_libraries(${_testname} ${QT_QTTEST_LIBRARY} kcoreaddons)
               if(WINCE)
                target_link_libraries(${_testname} ${WCECOMPAT_LIBRARIES})
               endif(WINCE)
       ENDFOREACH(_testname)
ENDMACRO(KCOREADDONS_EXECUTABLE_TESTS)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/kernel
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/text
    ${CMAKE_CURRENT_BINARY_DIR}/../src
)

KCOREADDONS_UNIT_TESTS(
    kaboutdatatest
    kcharsetstest
)


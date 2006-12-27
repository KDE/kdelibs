IF(KDE4_BUILD_TESTS)
    # Tests don't need to go into toplevel/bin, they are fine in the current dir.
    set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

    MACRO(PHONON_BUILD_TESTS)
        FOREACH(_testname ${ARGN})
            kde4_automoc(${_testname}.cpp)
            kde4_add_executable(${_testname} ${_testname}.cpp)
            target_link_libraries(${_testname} kalsadevicelist ${QT_QTTEST_LIBRARY})
        ENDFOREACH(_testname)
    ENDMACRO(PHONON_BUILD_TESTS)
    MACRO(PHONON_EXECUTE_TESTS)
        FOREACH(_testname ${ARGN})
            add_test(${_testname} ${EXECUTABLE_OUTPUT_PATH}/${_testname})
        ENDFOREACH(_testname)
    ENDMACRO(PHONON_EXECUTE_TESTS)

    PHONON_BUILD_TESTS(
        simpletest
        )
    PHONON_EXECUTE_TESTS(
        simpletest
        )
ENDIF(KDE4_BUILD_TESTS)

# vim: sw=4 ts=4 et tw=100

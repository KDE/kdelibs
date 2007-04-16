if(KDE4_BUILD_TESTS)

# Tests don't need to go into toplevel/bin, they are fine in the current dir.
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

MACRO(PHONON_BUILD_TESTS)
	FOREACH(_testname ${ARGN})
		kde4_automoc(${_testname}.cpp)
		kde4_add_executable(${_testname} NOGUI ${_testname}.cpp)
		target_link_libraries(${_testname} ${KDE4_PHONONUI_LIBS} ${QT_QTTEST_LIBRARY})
	ENDFOREACH(_testname)
ENDMACRO(PHONON_BUILD_TESTS)

# same as above but using the fake backend
MACRO(PHONON_BUILD_TESTS_FAKE)
	FOREACH(_testname ${ARGN})
		kde4_automoc(${_testname}.cpp)
		kde4_add_executable(${_testname}_fake NOGUI ${_testname}.cpp)
		target_link_libraries(${_testname}_fake ${KDE4_PHONONUI_LIBS} ${QT_QTTEST_LIBRARY})
		SET_TARGET_PROPERTIES(${_testname}_fake PROPERTIES COMPILE_FLAGS -DUSE_FAKE_BACKEND)
	ENDFOREACH(_testname)
ENDMACRO(PHONON_BUILD_TESTS_FAKE)

MACRO(PHONON_EXECUTE_TESTS)
	FOREACH(_testname ${ARGN})
		add_test(${_testname} ${EXECUTABLE_OUTPUT_PATH}/${_testname})
	ENDFOREACH(_testname)
ENDMACRO(PHONON_EXECUTE_TESTS)

PHONON_BUILD_TESTS(
	seekslider
	guitest
	mediaplayer
	uimethodtest
	)
PHONON_BUILD_TESTS_FAKE(
	seekslider
	uimethodtest
	)
PHONON_EXECUTE_TESTS(
	seekslider_fake
	uimethodtest_fake
	)

endif(KDE4_BUILD_TESTS)

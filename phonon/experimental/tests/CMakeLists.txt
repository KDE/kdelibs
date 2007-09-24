set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(${KDE4_KIO_INCLUDES})

MACRO(PHONON_UNIT_TESTS)
	FOREACH(_testname ${ARGN})
		kde4_add_unit_test(${_testname} ${_testname}.cpp)
		target_link_libraries(${_testname} ${KDE4_PHONONUI_LIBS} phononexperimental ${QT_QTTEST_LIBRARY})
	ENDFOREACH(_testname)
ENDMACRO(PHONON_UNIT_TESTS)

# same as above but using the fake backend
MACRO(PHONON_UNIT_TESTS_FAKE)
	FOREACH(_testname ${ARGN})
		kde4_add_unit_test(${_testname}_fake ${_testname}.cpp)
		target_link_libraries(${_testname}_fake ${KDE4_PHONONUI_LIBS} phononexperimental ${QT_QTTEST_LIBRARY})
		SET_TARGET_PROPERTIES(${_testname}_fake PROPERTIES COMPILE_FLAGS -DUSE_FAKE_BACKEND)
	ENDFOREACH(_testname)
ENDMACRO(PHONON_UNIT_TESTS_FAKE)

MACRO(PHONON_EXECUTABLE_TESTS)
	FOREACH(_testname ${ARGN})
		kde4_add_executable(${_testname} TEST ${_testname}.cpp)
		target_link_libraries(${_testname} ${KDE4_PHONONUI_LIBS} phononexperimental ${QT_QTTEST_LIBRARY})

	ENDFOREACH(_testname)
ENDMACRO(PHONON_EXECUTABLE_TESTS)

PHONON_EXECUTABLE_TESTS(
	audiodataoutputtest
	)

PHONON_UNIT_TESTS_FAKE(
	audiodataoutputtest
	)

# vim: noet sw=8

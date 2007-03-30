add_subdirectory(fakebackend)

include_directories(${KDE4_KIO_INCLUDES})

IF(KDE4_BUILD_TESTS)

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
	add_definitions(-DUSE_FAKE_BACKEND)
	FOREACH(_testname ${ARGN})
		kde4_automoc(${_testname}.cpp)
		kde4_add_executable(${_testname}_fake NOGUI ${_testname}.cpp)
		target_link_libraries(${_testname}_fake ${KDE4_PHONONUI_LIBS} ${QT_QTTEST_LIBRARY})
	ENDFOREACH(_testname)
ENDMACRO(PHONON_BUILD_TESTS_FAKE)

MACRO(PHONON_EXECUTE_TESTS)
	FOREACH(_testname ${ARGN})
		add_test(${_testname} ${EXECUTABLE_OUTPUT_PATH}/${_testname})
	ENDFOREACH(_testname)
ENDMACRO(PHONON_EXECUTE_TESTS)

PHONON_BUILD_TESTS(
	methodtest
	mediaobjecttest
	audiopathtest
	backendcapabilitiestest
	audiooutputtest
	audiodataoutputtest
	audiooutputdevicetest
	audiocapturedevicetest
	videocapturedevicetest
	bytestreamtest
	bytestreamtest2
	audiodevicelisttest
	)
target_link_libraries(audiodevicelisttest ${KDE4_KAUDIODEVICELIST_LIBS})

PHONON_BUILD_TESTS_FAKE(
	methodtest
	mediaobjecttest
	audiopathtest
	backendcapabilitiestest
	audiooutputtest
	audiodataoutputtest
	audiooutputdevicetest
	audiocapturedevicetest
	videocapturedevicetest
	bytestreamtest
	bytestreamtest2
	)
PHONON_EXECUTE_TESTS(
	methodtest_fake
	mediaobjecttest_fake
	audiopathtest_fake
	backendcapabilitiestest_fake
	audiooutputtest_fake
	audiodataoutputtest_fake
	audiooutputdevicetest_fake
	audiocapturedevicetest_fake
	videocapturedevicetest_fake
	#bytestreamtest_fake
	bytestreamtest2_fake
	audiodevicelisttest
	)

ENDIF(KDE4_BUILD_TESTS)

# vim: noet sw=8

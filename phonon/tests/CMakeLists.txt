add_subdirectory(fakebackend)

IF(KDE4_BUILD_TESTS)

# Tests don't need to go into toplevel/bin, they are fine in the current dir.
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

MACRO(PHONON_BUILD_TESTS)
	FOREACH(_testname ${ARGN})
		kde4_automoc(${_testname}.cpp)
		kde4_add_executable(${_testname} NOGUI ${_testname}.cpp)
		target_link_libraries(${_testname} ${KDE4_PHONONCORE_LIBS} ${QT_QTTEST_LIBRARY})
	ENDFOREACH(_testname)
ENDMACRO(PHONON_BUILD_TESTS)
MACRO(PHONON_EXECUTE_TESTS)
	FOREACH(_testname ${ARGN})
		#add_test(${_testname} ${EXECUTABLE_OUTPUT_PATH}/${_testname})
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
PHONON_EXECUTE_TESTS(
	methodtest
	backendcapabilitiestest
	audiopathtest
	audiooutputtest
	audiooutputdevicetest
	audiocapturedevicetest
	videocapturedevicetest
	bytestreamtest2
	audiodevicelisttest
	)
ENDIF(KDE4_BUILD_TESTS)

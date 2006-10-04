
add_subdirectory( fakebackend ) 

MACRO( PHONON_DECLARE_TESTS )
	IF(KDE4_BUILD_TESTS)
		FOREACH( _testname ${ARGN} )
			kde4_automoc( ${_testname}.cpp )
			kde4_add_executable( ${_testname} ${_testname}.cpp )
			target_link_libraries( ${_testname} ${KDE4_KIO_LIBS} phononcore ${QT_QTTEST_LIBRARY} )
		ENDFOREACH( _testname )
	ENDIF(KDE4_BUILD_TESTS)
ENDMACRO( PHONON_DECLARE_TESTS )

PHONON_DECLARE_TESTS(
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
	)
if(KDE4_BUILD_TESTS)
  add_test(methodtest ${EXECUTABLE_OUTPUT_PATH}/methodtest)
  add_test(backendcapabilitiestest ${EXECUTABLE_OUTPUT_PATH}/backendcapabilitiestest)
  add_test(audiopathtest ${EXECUTABLE_OUTPUT_PATH}/audiopathtest)
  add_test(audiooutputtest ${EXECUTABLE_OUTPUT_PATH}/audiooutputtest)
  add_test(audiooutputdevicetest ${EXECUTABLE_OUTPUT_PATH}/audiooutputdevicetest)
  add_test(audiocapturedevicetest ${EXECUTABLE_OUTPUT_PATH}/audiocapturedevicetest)
  add_test(videocapturedevicetest ${EXECUTABLE_OUTPUT_PATH}/videocapturedevicetest)
endif(KDE4_BUILD_TESTS)

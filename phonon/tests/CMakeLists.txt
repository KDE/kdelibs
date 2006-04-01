kde4_header()

add_subdirectory( fakebackend ) 

MACRO( PHONON_DECLARE_TESTS )
	IF(KDE4_BUILD_TESTS)
		FOREACH( _testname ${ARGN} )
			kde4_automoc( ${_testname}.cpp )
			kde4_add_executable( ${_testname} ${_testname}.cpp )
			target_link_libraries( ${_testname} ${KDE4_KDECORE_LIBS} phononcore ${QT_QTTEST_LIBRARY} )
		ENDFOREACH( _testname )
	ENDIF(KDE4_BUILD_TESTS)
ENDMACRO( PHONON_DECLARE_TESTS )

PHONON_DECLARE_TESTS(
	mediaobjecttest
	audiopathtest
	backendcapabilitiestest
	audiooutputtest
	audiodataoutputtest
	audiooutputdevicetest
	)

kde4_footer()

add_subdirectory(fakebackend)

include_directories(${KDE4_KIO_INCLUDES} ${CMAKE_SOURCE_DIR}/solid)

IF(KDE4_BUILD_TESTS)

# Tests don't need to go into toplevel/bin, they are fine in the current dir.
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

MACRO(PHONON_BUILD_TESTS)
	FOREACH(_testname ${ARGN})
		kde4_automoc(${_testname}.cpp)
		kde4_add_executable(${_testname} NOGUI ${_testname}.cpp)
		target_link_libraries(${_testname} ${KDE4_PHONON_LIBS} ${QT_QTTEST_LIBRARY})
	ENDFOREACH(_testname)
ENDMACRO(PHONON_BUILD_TESTS)

# same as above but using the fake backend
MACRO(PHONON_BUILD_TESTS_FAKE)
	FOREACH(_testname ${ARGN})
		kde4_automoc(${_testname}.cpp)
		kde4_add_executable(${_testname}_fake NOGUI ${_testname}.cpp)
		target_link_libraries(${_testname}_fake ${KDE4_PHONON_LIBS} ${QT_QTTEST_LIBRARY})
		SET_TARGET_PROPERTIES(${_testname}_fake PROPERTIES COMPILE_FLAGS -DUSE_FAKE_BACKEND)
	ENDFOREACH(_testname)
ENDMACRO(PHONON_BUILD_TESTS_FAKE)

MACRO(PHONON_EXECUTE_TESTS)
	FOREACH(_testname ${ARGN})
		add_test(${_testname} ${EXECUTABLE_OUTPUT_PATH}/${_testname})
	ENDFOREACH(_testname)
ENDMACRO(PHONON_EXECUTE_TESTS)

PHONON_BUILD_TESTS(
	mediaobjecttest
	audiopathtest
	backendcapabilitiestest
	audiooutputtest
	audiooutputdevicetest
	audiocapturedevicetest
	videocapturedevicetest
	#bytestreamtest
	#bytestreamtest2
	audiodevicelisttest
	seekslidertest
	volumeslidertest
	guitest
	mediaplayer
	)
target_link_libraries(backendcapabilitiestest ${KDE4_KDECORE_LIBS})
target_link_libraries(audiodevicelisttest ${KDE4_KAUDIODEVICELIST_LIBS})
target_link_libraries(mediaplayer ${KDE4_KDEUI_LIBS})
target_link_libraries(guitest ${KDE4_KIO_LIBS})

PHONON_BUILD_TESTS_FAKE(
	mediaobjecttest
	audiopathtest
	backendcapabilitiestest
	audiooutputtest
	audiooutputdevicetest
	audiocapturedevicetest
	videocapturedevicetest
	#bytestreamtest
	#bytestreamtest2
	seekslidertest
	volumeslidertest
	)
target_link_libraries(backendcapabilitiestest_fake ${KDE4_KDECORE_LIBS})
PHONON_EXECUTE_TESTS(
	mediasourcetest
	audiopathtest_fake
	backendcapabilitiestest_fake
	audiooutputtest_fake
	audiooutputdevicetest_fake
	audiocapturedevicetest_fake
	videocapturedevicetest_fake
	audiodevicelisttest
	seekslidertest_fake
	volumeslidertest_fake
	mediaobjecttest_fake
	#bytestreamtest_fake
	#bytestreamtest2_fake
	)

#methodtest does not link against libphonon
kde4_automoc(methodtest.cpp ../factory.cpp)
kde4_add_executable(methodtest NOGUI methodtest.cpp ../factory.cpp ../phononnamespace.cpp)
target_link_libraries(methodtest ${QT_QTDBUS_LIBRARY} ${QT_QTCORE_LIBRARY}  ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY})
SET_TARGET_PROPERTIES(methodtest PROPERTIES COMPILE_FLAGS "-DPHONON_METHODTEST -DPHONON_EXPORT=")
kde4_add_executable(methodtest_fake NOGUI methodtest.cpp ../factory.cpp ../phononnamespace.cpp)
target_link_libraries(methodtest_fake ${QT_QTDBUS_LIBRARY} ${QT_QTCORE_LIBRARY}  ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY})
SET_TARGET_PROPERTIES(methodtest_fake PROPERTIES COMPILE_FLAGS "-DUSE_FAKE_BACKEND -DPHONON_METHODTEST -DPHONON_EXPORT=")
add_test(methodtest_fake ${EXECUTABLE_OUTPUT_PATH}/methodtest_fake)

SET(mediasourcetest_SRCS mediasourcetest.cpp)
QT4_ADD_RESOURCES(mediasourcetest_SRCS testresources.qrc)
kde4_automoc(${mediasourcetest_SRCS})
kde4_add_executable(mediasourcetest NOGUI ${mediasourcetest_SRCS})
target_link_libraries(mediasourcetest ${KDE4_PHONON_LIBS} ${QT_QTTEST_LIBRARY})

SET(backendtester_SRCS backendtester.cpp)
kde4_add_ui_files(backendtester_SRCS backendtester.ui)
kde4_automoc(${backendtester_SRCS})
kde4_add_executable(backendtester ${backendtester_SRCS})
target_link_libraries(backendtester ${KDE4_KDEUI_LIBS})

ENDIF(KDE4_BUILD_TESTS)

# vim: noet sw=8

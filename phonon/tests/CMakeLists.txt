add_subdirectory(fakebackend)

include_directories(${KDE4_KIO_INCLUDES} ${CMAKE_SOURCE_DIR}/solid)

MACRO(PHONON_UNIT_TESTS)
	FOREACH(_testname ${ARGN})
		kde4_automoc(${_testname}.cpp)
		kde4_add_unit_test(${_testname} ${_testname}.cpp)
		target_link_libraries(${_testname} ${KDE4_PHONON_LIBS} ${QT_QTTEST_LIBRARY})
	ENDFOREACH(_testname)
ENDMACRO(PHONON_UNIT_TESTS)

# same as above but using the fake backend
MACRO(PHONON_UNIT_TESTS_FAKE)
	FOREACH(_testname ${ARGN})
		kde4_automoc(${_testname}.cpp)
		kde4_add_unit_test(${_testname}_fake ${_testname}.cpp)
		target_link_libraries(${_testname}_fake ${KDE4_PHONON_LIBS} ${QT_QTTEST_LIBRARY})
		SET_TARGET_PROPERTIES(${_testname}_fake PROPERTIES COMPILE_FLAGS -DUSE_FAKE_BACKEND)
	ENDFOREACH(_testname)
ENDMACRO(PHONON_UNIT_TESTS_FAKE)

MACRO(PHONON_EXECUTABLE_TESTS)
	FOREACH(_testname ${ARGN})
		kde4_automoc(${_testname}.cpp)
		kde4_add_test_executable(${_testname} ${_testname}.cpp)
		target_link_libraries(${_testname} ${KDE4_PHONON_LIBS} ${QT_QTTEST_LIBRARY})
	ENDFOREACH(_testname)
ENDMACRO(PHONON_EXECUTABLE_TESTS)

PHONON_EXECUTABLE_TESTS(
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
	guitest
	mediaplayer
	)
PHONON_UNIT_TESTS(
	audiodevicelisttest
	)
PHONON_UNIT_TESTS_FAKE(
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
target_link_libraries(backendcapabilitiestest ${KDE4_KDECORE_LIBS})
target_link_libraries(audiodevicelisttest ${KDE4_KAUDIODEVICELIST_LIBS})
target_link_libraries(mediaplayer ${KDE4_KDEUI_LIBS})
target_link_libraries(guitest ${KDE4_KIO_LIBS})
target_link_libraries(backendcapabilitiestest_fake ${KDE4_KDECORE_LIBS})
#methodtest does not link against libphonon
kde4_automoc(methodtest.cpp ../factory.cpp)
kde4_add_test_executable(methodtest methodtest.cpp ../factory.cpp ../phononnamespace.cpp)
target_link_libraries(methodtest ${QT_QTDBUS_LIBRARY} ${QT_QTCORE_LIBRARY}  ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY})
SET_TARGET_PROPERTIES(methodtest PROPERTIES COMPILE_FLAGS "-DPHONON_METHODTEST -DPHONON_EXPORT=")
kde4_add_unit_test(methodtest_fake methodtest.cpp ../factory.cpp ../phononnamespace.cpp)
target_link_libraries(methodtest_fake ${QT_QTDBUS_LIBRARY} ${QT_QTCORE_LIBRARY}  ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY})
SET_TARGET_PROPERTIES(methodtest_fake PROPERTIES COMPILE_FLAGS "-DUSE_FAKE_BACKEND -DPHONON_METHODTEST -DPHONON_EXPORT=")

SET(mediasourcetest_SRCS mediasourcetest.cpp)
QT4_ADD_RESOURCES(mediasourcetest_SRCS testresources.qrc)
kde4_automoc(${mediasourcetest_SRCS})
kde4_add_unit_test(mediasourcetest ${mediasourcetest_SRCS})
target_link_libraries(mediasourcetest ${KDE4_PHONON_LIBS} ${QT_QTTEST_LIBRARY})

SET(backendtester_SRCS backendtester.cpp)
kde4_add_ui_files(backendtester_SRCS backendtester.ui)
kde4_automoc(${backendtester_SRCS})
kde4_add_test_executable(backendtester ${backendtester_SRCS})
target_link_libraries(backendtester ${KDE4_KDEUI_LIBS})

# vim: noet sw=8

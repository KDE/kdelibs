set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include(ECMMarkAsTest)

MACRO(KIO_UNIT_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_add_unit_test(${_testname} TESTNAME kio-${_testname} ${_testname}.cpp)
               target_link_libraries(${_testname} ${KDE4_KIO_LIBS} Qt5::Test KIconThemes XmlGui ${KCoreAddons_LIBRARIES} KI18n)
       ENDFOREACH(_testname)
ENDMACRO(KIO_UNIT_TESTS)
MACRO(KIO_EXECUTABLE_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_add_executable(${_testname} TEST ${_testname}.cpp)
               target_link_libraries(${_testname} ${KDE4_KIO_LIBS} Qt5::Test kdeqt5staging XmlGui ${KCoreAddons_LIBRARIES} KI18n KIconThemes)
       ENDFOREACH(_testname)
ENDMACRO(KIO_EXECUTABLE_TESTS)

# jobtest seems to trigger a ctest problem; jobtest finishes and ctest waits for ever

KIO_UNIT_TESTS(
  kprotocolinfotest
  jobtest
  jobguitest
  jobremotetest
  kurlcompletiontest
  kfileitemtest
  kbookmarktest
  kacltest
  kdirmodeltest
  kdirlistertest
  fileundomanagertest
  dataprotocoltest
  clipboardupdatertest
)

KIO_EXECUTABLE_TESTS(
  ksycocatest
  getalltest
  kioslavetest
  speed
  kdirlistertest_gui
  previewtest
  kionetrctest
  kpropsdlgtest
  ksycocaupdatetest
  netaccesstest
  kdirmodeltest_gui
  kmimetypechoosertest_gui
  kdbusservicestartertest
  kurlrequestertest
  kurlrequestertest_gui
  kopenwithtest
  kfiledialogtest
  kbookmarkdialogtest
)

if(UNIX)
 KIO_UNIT_TESTS(
    klocalsockettest
    klocalsocketservertest
 )
endif(UNIX)

########### kfstest ###############

set(kfstest_SRCS kfstest.cpp kfdtest.cpp)
add_executable(kfstest ${kfstest_SRCS})
ecm_mark_as_test(kfstest)
target_link_libraries(kfstest KIconThemes ${KDE4_KIO_LIBS} ${KCoreAddons_LIBRARIES} KWidgetsAddons)


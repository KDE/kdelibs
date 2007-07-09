# for <kio/global.h>
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. )

MACRO(KIO_UNIT_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_add_unit_test(${_testname} ${_testname}.cpp)
               target_link_libraries(${_testname} ${KDE4_KIO_LIBS} ${QT_QTTEST_LIBRARY})
       ENDFOREACH(_testname)
ENDMACRO(KIO_UNIT_TESTS)
MACRO(KIO_EXECUTABLE_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_add_test_executable(${_testname} ${_testname}.cpp)
               target_link_libraries(${_testname} ${KDE4_KIO_LIBS} ${QT_QTTEST_LIBRARY})
       ENDFOREACH(_testname)
ENDMACRO(KIO_EXECUTABLE_TESTS)

# jobtest seems to trigger a ctest problem; jobtest finishes and ctest waits for ever

KIO_UNIT_TESTS(
  karchivetest
  krununittest
  kprotocolinfotest
  jobtest
  kurlcompletiontest
  kfileitemtest
  kbookmarktest
  kacltest
  kdirmodeltest
  kdirlistertest
  kmountpointtest
)

KIO_EXECUTABLE_TESTS(
  ksycocatest
  getalltest
  kruntest
  ktartest
  kziptest
  kioslavetest
  kdirwatchtest
  speed
  kiopassdlgtest
  kscantest
  safesitetest
  kdirlistertest_gui
  previewtest
  kionetrctest
  kpropsdlgtest
  kmfitest
  dataprotocoltest
  ksycocaupdatetest
  netaccesstest
  kdirwatchtest_gui
  kdirmodeltest_gui
  kmimetypechoosertest_gui
  ksslkeygentest
)


set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
# for <kio/global.h>
include_directories( ${KDE4_KIO_INCLUDES} )

MACRO(KIO_UNIT_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_add_unit_test(${_testname} TESTNAME kio-${_testname} ${_testname}.cpp)
               target_link_libraries(${_testname} ${KDE4_KIO_LIBS} ${QT_QTTEST_LIBRARY})
       ENDFOREACH(_testname)
ENDMACRO(KIO_UNIT_TESTS)
MACRO(KIO_EXECUTABLE_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_add_executable(${_testname} TEST ${_testname}.cpp)
               target_link_libraries(${_testname} ${KDE4_KIO_LIBS} ${QT_QTTEST_LIBRARY})
       ENDFOREACH(_testname)
ENDMACRO(KIO_EXECUTABLE_TESTS)

# jobtest seems to trigger a ctest problem; jobtest finishes and ctest waits for ever

KIO_UNIT_TESTS(
  krununittest
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
  kfilemetainfotest
  dataprotocoltest
  clipboardupdatertest
)

KIO_EXECUTABLE_TESTS(
  ksycocatest
  getalltest
  kruntest
  kioslavetest
  speed
  kscantest
  kdirlistertest_gui
  previewtest
  kionetrctest
  kpropsdlgtest
  kmfitest
  ksycocaupdatetest
  netaccesstest
  kdirmodeltest_gui
  kmimetypechoosertest_gui
  ksslkeygentest
  kdbusservicestartertest
  kurlrequestertest
  kopenwithtest
  kicondialogtest
  kfiledialogtest
)

if(NOT KDE_NO_DEPRECATED)
KIO_EXECUTABLE_TESTS(
  kiopassdlgtest
)
endif(NOT KDE_NO_DEPRECATED)

########### kfstest ###############

set(kfstest_SRCS kfstest.cpp kfdtest.cpp)
kde4_add_executable(kfstest TEST ${kfstest_SRCS})
target_link_libraries(kfstest  ${KDE4_KIO_LIBS} )


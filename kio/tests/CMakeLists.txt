# Tests don't need to go into toplevel/bin, they are fine in the current dir.
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

# for <kio/global.h>
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. )

########## ktradertest - which is installed

kde4_add_executable(ktradertest NOGUI ktradertest.cpp)

target_link_libraries(ktradertest ${KDE4_KIO_LIBS})

install(TARGETS ktradertest DESTINATION ${BIN_INSTALL_DIR} )

########### TESTS ###############

if(KDE4_BUILD_TESTS)

MACRO(KIO_BUILD_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_automoc(${_testname}.cpp)
               kde4_add_executable(${_testname} NOGUI RUN_UNINSTALLED ${_testname}.cpp)
               target_link_libraries(${_testname} ${KDE4_KIO_LIBS} ${QT_QTTEST_LIBRARY})
       ENDFOREACH(_testname)
ENDMACRO(KIO_BUILD_TESTS)
MACRO(KIO_EXECUTE_TESTS)
       FOREACH(_testname ${ARGN})
               add_test(kio-${_testname} ${EXECUTABLE_OUTPUT_PATH}/${_testname})
       ENDFOREACH(_testname)
ENDMACRO(KIO_EXECUTE_TESTS)

# jobtest seems to trigger a ctest problem; jobtest finishes and ctest waits for ever

set(kio_unittests
  karchivetest
  krununittest
  kprotocolinfotest
  jobtest
  kurlcompletiontest
  kfileitemtest
  kbookmarktest
  kacltest
  kdirmodeltest
)
 
KIO_BUILD_TESTS(
  ${kio_unittests}
  ksycocatest
  getalltest
  kruntest
  ktartest
  kziptest
  kioslavetest
  kdirwatchtest
  speed
  kdefaultprogresstest
  kiopassdlgtest
  kscantest
  safesitetest
  kdirlistertest
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
)

KIO_EXECUTE_TESTS(
  ${kio_unittests}
)

###########  ###############

endif(KDE4_BUILD_TESTS)

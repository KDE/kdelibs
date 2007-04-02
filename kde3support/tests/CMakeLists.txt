# Tests don't need to go into toplevel/bin, they are fine in the current dir.
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${CMAKE_SOURCE_DIR}/kde3support ${CMAKE_SOURCE_DIR}/kde3support/kdeui )

MACRO(KDE3SUPPORT_BUILD_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_automoc(${_testname}.cpp)
               kde4_add_test(${_testname} ${_testname}.cpp)
               target_link_libraries(${_testname} ${KDE4_KDE3SUPPORT_LIBS} ${QT_QTTEST_LIBRARY})
       ENDFOREACH(_testname)
ENDMACRO(KDE3SUPPORT_BUILD_TESTS)
MACRO(KDE3SUPPORT_EXECUTE_TESTS)
       FOREACH(_testname ${ARGN})
               add_test(kde3support-${_testname} ${EXECUTABLE_OUTPUT_PATH}/${_testname})
       ENDFOREACH(_testname)
ENDMACRO(KDE3SUPPORT_EXECUTE_TESTS)

set(kde3support_unittests
  kcommandtest
  krfcdatetest
)

KDE3SUPPORT_BUILD_TESTS(
  itemcontainertest
  k3listviewtest
  kactivelabeltest
  kdockwidgetdemo
  kdockwidgettest
  kdocktest
  kaboutdialogtest
  ${kde3support_unittests}
)

KDE3SUPPORT_EXECUTE_TESTS(
  ${kde3support_unittests}
)

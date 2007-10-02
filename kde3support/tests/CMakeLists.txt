set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${CMAKE_SOURCE_DIR}/kde3support ${CMAKE_SOURCE_DIR}/kde3support/kdeui )

MACRO(KDE3SUPPORT_UNIT_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_add_unit_test(${_testname} ${_testname}.cpp)
               target_link_libraries(${_testname} ${KDE4_KDE3SUPPORT_LIBS} ${QT_QTTEST_LIBRARY})
       ENDFOREACH(_testname)
ENDMACRO(KDE3SUPPORT_UNIT_TESTS)
MACRO(KDE3SUPPORT_EXECUTABLE_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_add_executable(${_testname} TEST ${_testname}.cpp)
               target_link_libraries(${_testname} ${KDE4_KDE3SUPPORT_LIBS} ${QT_QTTEST_LIBRARY})
       ENDFOREACH(_testname)
ENDMACRO(KDE3SUPPORT_EXECUTABLE_TESTS)

KDE3SUPPORT_UNIT_TESTS(
  kcommandtest
  krfcdatetest
)

KDE3SUPPORT_EXECUTABLE_TESTS(
  itemcontainertest
  k3listviewtest
  kactivelabeltest
  kdockwidgetdemo
  kdockwidgettest
  kdocktest
  kaboutdialogtest
  k3processtest
  k3prociotest
)


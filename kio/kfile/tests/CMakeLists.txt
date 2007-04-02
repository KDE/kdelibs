# Tests don't need to go into toplevel/bin, they are fine in the current dir.
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${KDE4_KIO_INCLUDES} )

MACRO(KIO_BUILD_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_automoc(${_testname}.cpp)
               kde4_add_test(${_testname} ${_testname}.cpp)
               target_link_libraries(${_testname} ${KDE4_KIO_LIBS} ${QT_QTTEST_LIBRARY})
       ENDFOREACH(_testname)
ENDMACRO(KIO_BUILD_TESTS)
MACRO(KIO_EXECUTE_TESTS)
       FOREACH(_testname ${ARGN})
               add_test(kio-${_testname} ${EXECUTABLE_OUTPUT_PATH}/${_testname})
       ENDFOREACH(_testname)
ENDMACRO(KIO_EXECUTE_TESTS)

KIO_BUILD_TESTS(
  kurlrequestertest
  kopenwithtest
  kicondialogtest
)

########### next target ###############

set(kfstest_SRCS kfstest.cpp kfdtest.cpp)
kde4_automoc(${kfstest_SRCS})
kde4_add_test(kfstest ${kfstest_SRCS})
target_link_libraries(kfstest  ${KDE4_KIO_LIBS} )

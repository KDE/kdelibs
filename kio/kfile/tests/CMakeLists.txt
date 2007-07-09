include_directories( ${KDE4_KIO_INCLUDES} )

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

KIO_EXECUTABLE_TESTS(
  kurlrequestertest
  kopenwithtest
  kicondialogtest
)

########### next target ###############

set(kfstest_SRCS kfstest.cpp kfdtest.cpp)
kde4_add_test_executable(kfstest ${kfstest_SRCS})
target_link_libraries(kfstest  ${KDE4_KIO_LIBS} )

MACRO(KFILE_UNIT_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_add_unit_test(${_testname} ${_testname}.cpp)
               target_link_libraries(${_testname} ${KDE4_KFILE_LIBS} ${QT_QTTEST_LIBRARY})
       ENDFOREACH(_testname)
ENDMACRO(KFILE_UNIT_TESTS)
MACRO(KFILE_EXECUTABLE_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_add_test_executable(${_testname} ${_testname}.cpp)
               target_link_libraries(${_testname} ${KDE4_KFILE_LIBS} ${QT_QTTEST_LIBRARY})
       ENDFOREACH(_testname)
ENDMACRO(KFILE_EXECUTABLE_TESTS)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. )

KFILE_EXECUTABLE_TESTS(
  kdirselectdialogtest
  kfiletreeviewtest
)


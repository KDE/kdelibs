MACRO(KFILE_BUILD_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_automoc(${_testname}.cpp)
               kde4_add_test(${_testname} ${_testname}.cpp)
               target_link_libraries(${_testname} ${KDE4_KFILE_LIBS} ${QT_QTTEST_LIBRARY})
       ENDFOREACH(_testname)
ENDMACRO(KFILE_BUILD_TESTS)
MACRO(KFILE_EXECUTE_TESTS)
       FOREACH(_testname ${ARGN})
               add_test(kfile-${_testname} ${EXECUTABLE_OUTPUT_PATH}/${_testname})
       ENDFOREACH(_testname)
ENDMACRO(KFILE_EXECUTE_TESTS)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. )

KFILE_BUILD_TESTS(
  kdirselectdialogtest
  kfiletreeviewtest
)


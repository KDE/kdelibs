set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

########### next target ###############

# TODO automate and merge with test_filter?
set(test_suggest_SRCS test.cpp)

kde4_add_executable(test_suggest TEST ${test_suggest_SRCS})

target_link_libraries(test_suggest ${KDE4_KDECORE_LIBS} )

########### unittests ###############

MACRO(SONNET_UNIT_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_add_unit_test(${_testname} TESTNAME sonnet-${_testname} ${_testname}.cpp)
               target_link_libraries(${_testname} ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} ${QT_QTXML_LIBRARY} ${KDEWIN_LIBRARIES})
       ENDFOREACH(_testname)
ENDMACRO(SONNET_UNIT_TESTS)

SONNET_UNIT_TESTS(
 test_filter
 test_core
)


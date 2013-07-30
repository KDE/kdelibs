set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include(ECMMarkAsTest)

MACRO(KDECORE_UNIT_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_add_unit_test(${_testname} TESTNAME "kdecore-${_testname}" NOGUI ${_testname}.cpp)
               target_link_libraries(${_testname} ${KDE4_KDECORE_LIBS} Qt5::Test Qt5::Network ${KCoreAddons_LIBRARIES} kdeqt5staging KDE4Support)
               if(WINCE)
                target_link_libraries(${_testname} ${WCECOMPAT_LIBRARIES})
               endif(WINCE)
       ENDFOREACH(_testname)
ENDMACRO(KDECORE_UNIT_TESTS)
MACRO(KDECORE_EXECUTABLE_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_add_executable(${_testname} NOGUI ${_testname}.cpp)
               ecm_mark_as_test(${_testname})
               target_link_libraries(${_testname} ${KDE4_KDECORE_LIBS} Qt5::Test ${KCoreAddons_LIBRARIES} )
               if(WINCE)
                target_link_libraries(${_testname} ${WCECOMPAT_LIBRARIES})
               endif(WINCE)
       ENDFOREACH(_testname)
ENDMACRO(KDECORE_EXECUTABLE_TESTS)


########### next target ###############

KDECORE_UNIT_TESTS(
 cplusplustest
 kautostarttest
 qcoreapptest
)

KDECORE_EXECUTABLE_TESTS(
 kmemtest
 dbuscalltest
)


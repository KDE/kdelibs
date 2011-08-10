set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../src
                     ${CMAKE_SOURCE_DIR}/kio/httpfilter
)

MACRO(KARCHIVE_UNIT_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_add_unit_test(${_testname} TESTNAME "karchive-${_testname}" NOGUI ${_testname}.cpp)
               target_link_libraries(${_testname} karchive ${QT_QTTEST_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${KDE4_KDECORE_LIBS})
               if(WINCE)
                target_link_libraries(${_testname} ${WCECOMPAT_LIBRARIES})
               endif(WINCE)
       ENDFOREACH(_testname)
ENDMACRO(KARCHIVE_UNIT_TESTS)
MACRO(KARCHIVE_EXECUTABLE_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_add_executable(${_testname} NOGUI TEST ${_testname}.cpp)
               target_link_libraries(${_testname} karchive ${QT_QTTEST_LIBRARY})
               if(WINCE)
                target_link_libraries(${_testname} ${WCECOMPAT_LIBRARIES})
               endif(WINCE)
       ENDFOREACH(_testname)
ENDMACRO(KARCHIVE_EXECUTABLE_TESTS)

KARCHIVE_UNIT_TESTS(
  karchivetest
)

KARCHIVE_EXECUTABLE_TESTS(
  ktartest
  kziptest
)

########### klimitediodevicetest ###############

kde4_add_unit_test(klimitediodevicetest TESTNAME karchive-klimitediodevicetest klimitediodevicetest.cpp ../src/klimitediodevice.cpp)
target_link_libraries(klimitediodevicetest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

########### kfiltertest ###############

# compile httpfilter.cpp into the test since it's not part of kdelibs
# (only par of kio_http and kmultipart)
set(kfiltertest_SRCS kfiltertest.cpp ${CMAKE_SOURCE_DIR}/kio/httpfilter/httpfilter.cc)
include_directories( ${CMAKE_SOURCE_DIR}/kio/httpfilter )
kde4_add_unit_test(kfiltertest TESTNAME karchive-kfiltertest ${kfiltertest_SRCS})
target_link_libraries(kfiltertest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} karchive)
target_link_libraries(kfiltertest ${ZLIB_LIBRARIES})

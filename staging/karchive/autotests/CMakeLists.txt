enable_testing()
set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/../src
   ${CMAKE_CURRENT_BINARY_DIR}/../src
   ${CMAKE_SOURCE_DIR}/kio/httpfilter
)

MACRO(KARCHIVE_UNIT_TESTS)
       FOREACH(_testname ${ARGN})
               add_executable(${_testname} ${_testname}.cpp) # TODO NOGUI
               add_test(karchive-${_testname} ${_testname})
               target_link_libraries(${_testname} karchive ${QT_QTTEST_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTCORE_LIBRARY} kcoreaddons)
               if(WINCE)
                target_link_libraries(${_testname} ${WCECOMPAT_LIBRARIES})
               endif(WINCE)
       ENDFOREACH(_testname)
ENDMACRO(KARCHIVE_UNIT_TESTS)
MACRO(KARCHIVE_EXECUTABLE_TESTS) # TODO move to ../examples
       FOREACH(_testname ${ARGN})
               add_executable(${_testname} ${_testname}.cpp) # TODO NOGUI
               target_link_libraries(${_testname} karchive ${QT_QTTEST_LIBRARY} ${QT_QTCORE_LIBRARY} )
               if(WINCE)
                target_link_libraries(${_testname} ${WCECOMPAT_LIBRARIES})
               endif(WINCE)
       ENDFOREACH(_testname)
ENDMACRO(KARCHIVE_EXECUTABLE_TESTS)

# TODO: QT5 Port to QFINDTESTDATA
add_definitions("-DKDESRCDIR=\"${CMAKE_CURRENT_SOURCE_DIR}\"")

KARCHIVE_UNIT_TESTS(
  karchivetest
)

KARCHIVE_EXECUTABLE_TESTS(
  ktartest
  kziptest
)

########### klimitediodevicetest ###############

add_executable(klimitediodevicetest klimitediodevicetest.cpp ../src/klimitediodevice.cpp)
add_test(karchive-klimitediodevicetest klimitediodevicetest)
target_link_libraries(klimitediodevicetest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

########### kfiltertest ###############

# compile httpfilter.cpp into the test since it's not part of kdelibs
# (only par of kio_http and kmultipart)
set(kfiltertest_SRCS kfiltertest.cpp ${CMAKE_SOURCE_DIR}/kio/httpfilter/httpfilter.cc)
include_directories( ${CMAKE_SOURCE_DIR}/kio/httpfilter )
add_executable(kfiltertest ${kfiltertest_SRCS})
add_test(karchive-kfiltertest kfiltertest)
target_link_libraries(kfiltertest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} karchive kcoreaddons kde4support)
target_link_libraries(kfiltertest ${ZLIB_LIBRARIES})

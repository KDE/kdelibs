remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/../src
   ${CMAKE_CURRENT_BINARY_DIR}/../src
)

include(ECMMarkAsTest)

macro(KARCHIVE_UNIT_TESTS)
   foreach(_testname ${ARGN})
      add_executable(${_testname} ${_testname}.cpp) # TODO NOGUI
      add_test(karchive-${_testname} ${_testname})
      target_link_libraries(${_testname} KArchive ${QT_QTTEST_LIBRARY} ${QT_QTNETWORK_LIBRARY})
      ecm_mark_as_test(${_testname})
   endforeach(_testname)
endmacro(KARCHIVE_UNIT_TESTS)

karchive_unit_tests(
  karchivetest
)

########### klimitediodevicetest ###############

add_executable(klimitediodevicetest klimitediodevicetest.cpp ../src/klimitediodevice.cpp)
add_test(karchive-klimitediodevicetest klimitediodevicetest)
target_link_libraries(klimitediodevicetest ${QT_QTTEST_LIBRARY} )

########### kfiltertest ###############

add_executable(kfiltertest kfiltertest.cpp)
add_test(karchive-kfiltertest kfiltertest)
target_link_libraries(kfiltertest ${QT_QTTEST_LIBRARY} KArchive ${ZLIB_LIBRARIES})

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/../src
   ${CMAKE_CURRENT_BINARY_DIR}/../src
)

#include(ECMMarkAsTest)

macro(KARCHIVE_UNIT_TESTS)
   foreach(_testname ${ARGN})
      add_executable(${_testname} ${_testname}.cpp) # TODO NOGUI
      add_test(karchive-${_testname} ${_testname})
      # TODO: QT5 remove KDESRCDIR, QFINDTESTDATA doesn't need it
      set_target_properties(${_testname} PROPERTIES COMPILE_FLAGS -DKDESRCDIR="\\"${CMAKE_CURRENT_SOURCE_DIR}/\\"")
      target_link_libraries(${_testname} karchive-qt4 ${QT_QTTEST_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTCORE_LIBRARY} ${inqt5_LIBRARY} ${QT_QTMAIN_LIBRARY})
      if(WINCE)
         target_link_libraries(${_testname} ${WCECOMPAT_LIBRARIES})
      endif()
      #ecm_mark_as_test(${_testname})
   endforeach(_testname)
endmacro(KARCHIVE_UNIT_TESTS)

karchive_unit_tests(
  karchivetest
)

########### klimitediodevicetest ###############

add_executable(klimitediodevicetest klimitediodevicetest.cpp ../src/klimitediodevice.cpp)
add_test(karchive-klimitediodevicetest klimitediodevicetest)
target_link_libraries(klimitediodevicetest ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY} ${QT_QTMAIN_LIBRARY} )

########### kfiltertest ###############

add_executable(kfiltertest kfiltertest.cpp)
add_test(karchive-kfiltertest kfiltertest)
target_link_libraries(kfiltertest ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY} ${QT_QTMAIN_LIBRARY} karchive-qt4 ${inqt5_LIBRARY} ${ZLIB_LIBRARIES})

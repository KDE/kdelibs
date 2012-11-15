remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/../src
   ${CMAKE_CURRENT_BINARY_DIR}/../src
   ${CMAKE_SOURCE_DIR}/kio/httpfilter
)

#include(ECMMarkAsTest)

macro(KARCHIVE_EXECUTABLE_TESTS)
   foreach(_testname ${ARGN})
      add_executable(${_testname} ${_testname}.cpp) # TODO NOGUI
      target_link_libraries(${_testname} karchive-qt4 ${QT_QTTEST_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTMAIN_LIBRARY} ${inqt5_LIBRARY})
      if(WINCE)
         target_link_libraries(${_testname} ${WCECOMPAT_LIBRARIES})
      endif()
      #ecm_mark_as_test(${_testname})
   endforeach(_testname)
endmacro(KARCHIVE_EXECUTABLE_TESTS)

karchive_executable_tests(
  ktartest
  kziptest
)

if(liblzma_found)
   karchive_executable_tests(
      k7ziptest
   )
endif()

enable_testing()
set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/../src
   ${CMAKE_CURRENT_BINARY_DIR}/../src
   ${CMAKE_SOURCE_DIR}/kio/httpfilter
)

include(ECMMarkAsTest)

MACRO(KARCHIVE_EXECUTABLE_TESTS)
       FOREACH(_testname ${ARGN})
               add_executable(${_testname} ${_testname}.cpp) # TODO NOGUI
               target_link_libraries(${_testname} karchive ${QT_QTTEST_LIBRARY} ${QT_QTCORE_LIBRARY} ${inqt5_LIBRARY})
               if(WINCE)
                target_link_libraries(${_testname} ${WCECOMPAT_LIBRARIES})
               endif(WINCE)
               ecm_mark_as_test(${_testname})
       ENDFOREACH(_testname)
ENDMACRO(KARCHIVE_EXECUTABLE_TESTS)

KARCHIVE_EXECUTABLE_TESTS(
  ktartest
  kziptest
)

if(LIBLZMA_FOUND)
        KARCHIVE_EXECUTABLE_TESTS(
          k7ziptest
        )
endif()

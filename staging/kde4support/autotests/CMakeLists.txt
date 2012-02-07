enable_testing()
set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/../src
   ${CMAKE_CURRENT_BINARY_DIR}/../src
   ${KDE4_KDECORE_INCLUDES}
)

MACRO(KDE4SUPPORT_UNIT_TESTS)
       FOREACH(_testname ${ARGN})
               add_executable(${_testname} ${_testname}.cpp) # TODO NOGUI
               add_test(kde4support-${_testname} ${_testname})
               target_link_libraries(${_testname} kde4support ${QT_QTTEST_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTCORE_LIBRARY} kdecore kcoreaddons)
               if(WINCE)
                target_link_libraries(${_testname} ${WCECOMPAT_LIBRARIES})
               endif(WINCE)
       ENDFOREACH(_testname)
ENDMACRO(KDE4SUPPORT_UNIT_TESTS)

KDE4SUPPORT_UNIT_TESTS(
  kmd5benchmark
  ktempdirtest
  ktemporaryfiletest
)


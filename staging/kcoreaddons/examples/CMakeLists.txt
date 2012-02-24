enable_testing()
set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/../src/kdecore
   ${CMAKE_CURRENT_BINARY_DIR}/../src
   ${KDE4_KDECORE_INCLUDES}
)

MACRO(KDE4SUPPORT_EXAMPLES)
       FOREACH(_testname ${ARGN})
               add_executable(${_testname} ${_testname}.cpp) # TODO NOGUI
               target_link_libraries(${_testname} kde4support ${QT_QTTEST_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTCORE_LIBRARY} kdecore kcoreaddons)
               if(WINCE)
                target_link_libraries(${_testname} ${WCECOMPAT_LIBRARIES})
               endif(WINCE)
       ENDFOREACH(_testname)
ENDMACRO()

KDE4SUPPORT_EXAMPLES(
  kdirwatchtest
  kcmdlineargstest
)


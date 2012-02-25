enable_testing()
set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/../src/randomness
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
  krandomsequencetest
)

########### kdirwatchtest_gui ###############

kde4_add_executable(kdirwatchtest_gui TEST kdirwatchtest_gui.cpp)
target_link_libraries(kdirwatchtest_gui ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY} kcoreaddons)

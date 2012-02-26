enable_testing()
set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/../src/randomness
   ${CMAKE_CURRENT_SOURCE_DIR}/../src/kdecore
   ${CMAKE_CURRENT_BINARY_DIR}/../src
   ${KDE4_KDECORE_INCLUDES}
)

MACRO(KCOREADDONS_EXAMPLES)
       FOREACH(_testname ${ARGN})
               add_executable(${_testname} ${_testname}.cpp) # TODO NOGUI
               target_link_libraries(${_testname} ${QT_QTTEST_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTCORE_LIBRARY} kcoreaddons)
               if(WINCE)
                target_link_libraries(${_testname} ${WCECOMPAT_LIBRARIES})
               endif(WINCE)
       ENDFOREACH(_testname)
ENDMACRO()

KCOREADDONS_EXAMPLES(
  kdirwatchtest
  kcmdlineargstest
  krandomsequencetest
)

########### kdirwatchtest_gui ###############

add_executable(kdirwatchtest_gui kdirwatchtest_gui.cpp)
target_link_libraries(kdirwatchtest_gui ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} kcoreaddons)


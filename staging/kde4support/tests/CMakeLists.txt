include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/kdecore
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/kdeui
  ${CMAKE_CURRENT_BINARY_DIR}/../src
  ${KDE4_KDECORE_INCLUDES}
)

include(ECMMarkAsTest)

macro(KDE4SUPPORT_EXAMPLES)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    target_link_libraries(${_testname} kde4support ${QT_QTTEST_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTMAIN_LIBRARY} kdecore kcoreaddons)
    if(WINCE)
      target_link_libraries(${_testname} ${WCECOMPAT_LIBRARIES})
    endif(WINCE)
    ecm_mark_as_test(${_testname})
  endforeach(_testname)
endmacro()

kde4support_examples(
  kmdcodectest
  kcmdlineargstest
  kdialogbuttonboxtest
  khboxtest
)


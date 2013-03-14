include_directories( ${KDE4_KDEUI_INCLUDES} )

include(ECMMarkAsTest)

macro(kwidgets_executable_tests)
   foreach(_testname ${ARGN})
      add_executable(${_testname} ${_testname}.cpp)
      target_link_libraries(${_testname} ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY} ${QT_QTXML_LIBRARY} ${KDEWIN_LIBRARIES} ${KCoreAddons_LIBRARIES} kde4support)
      ecm_mark_as_test(${_testname})
   endforeach()
endmacro()

kwidgets_executable_tests(
    kiconloadertest
    kiconeffecttest
    kmessagetest
    kmessageboxtest
)

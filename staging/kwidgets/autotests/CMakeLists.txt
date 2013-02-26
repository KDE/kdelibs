include_directories( ${KDE4_KDEUI_INCLUDES} )

include(ECMMarkAsTest)

macro(kwidgets_unit_tests)
   foreach(_testname ${ARGN})
      add_executable(${_testname} ${_testname}.cpp)
      add_test(kwidgets-${_testname} ${_testname})
      target_link_libraries(${_testname} ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTMAIN_LIBRARY} ${KDEWIN_LIBRARIES} ${KCoreAddons_LIBRARIES} )
      ecm_mark_as_test(${_testname})
   endforeach()
endmacro()

macro(kwidgets_executable_tests)
   foreach(_testname ${ARGN})
      add_executable(${_testname} ${_testname}.cpp)
      target_link_libraries(${_testname} ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTMAIN_LIBRARY} ${KDEWIN_LIBRARIES} ${KCoreAddons_LIBRARIES} )
      ecm_mark_as_test(${_testname})
   endforeach()
endmacro()

kwidgets_executable_tests(
    kglobalsettingsclient # helper program for kglobalsettingstest
)

kwidgets_unit_tests(
    kglobalsettingstest
    kiconloader_unittest
)

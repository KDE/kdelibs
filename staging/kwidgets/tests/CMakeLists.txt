
include(ECMMarkAsTest)

macro(kwidgets_executable_tests)
   foreach(_testname ${ARGN})
      add_executable(${_testname} ${_testname}.cpp)
      target_link_libraries(${_testname} ${KDE4_KDEUI_LIBS} Qt5::Test Qt5::Xml ${KDEWIN_LIBRARIES} ${KCoreAddons_LIBRARIES} kde4support)
      ecm_mark_as_test(${_testname})
   endforeach()
endmacro()

kwidgets_executable_tests(
    kcodecactiontest
    kmessagetest
    kmessageboxtest
    kselectactiontest
    kpagewidgettest
    kpagedialogtest
)

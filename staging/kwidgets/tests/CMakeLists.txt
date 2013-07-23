
include(ECMMarkAsTest)

macro(kwidgets_executable_tests)
   foreach(_testname ${ARGN})
      add_executable(${_testname} ${_testname}.cpp)
      target_link_libraries(${_testname} KWidgets Qt5::Widgets Qt5::Test Qt5::Xml XmlGui KCodecs ${KDE4_KDEUI_LIBS} KDE4Support) # dependence for kmessage
      ecm_mark_as_test(${_testname})
   endforeach()
endmacro()

kwidgets_executable_tests(
    kcodecactiontest
    kcolorcollectiontest
    klanguagebuttontest
    kmessagetest
    kmessageboxtest
    kselectactiontest
    openurltest
)

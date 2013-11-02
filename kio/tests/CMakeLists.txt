set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include(ECMMarkAsTest)
MACRO(KIO_EXECUTABLE_TESTS)
       FOREACH(_testname ${ARGN})
               add_executable(${_testname} ${_testname}.cpp)
               ecm_mark_as_test(${_testname})
               target_link_libraries(${_testname} ${KDE4_KIO_LIBS} Qt5::Test XmlGui KF5::KCoreAddons KF5::KI18n KF5::KIconThemes)
       ENDFOREACH(_testname)
ENDMACRO(KIO_EXECUTABLE_TESTS)

KIO_EXECUTABLE_TESTS(
  ksycocatest
  kionetrctest
  ksycocaupdatetest
  kdbusservicestartertest
)



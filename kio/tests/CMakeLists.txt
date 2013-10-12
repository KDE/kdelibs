set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include(ECMMarkAsTest)

MACRO(KIO_UNIT_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_add_unit_test(${_testname} TESTNAME kio-${_testname} ${_testname}.cpp)
               target_link_libraries(${_testname} ${KDE4_KIO_LIBS} Qt5::Test KF5::KIconThemes XmlGui KF5::KCoreAddons KF5::KI18n)
       ENDFOREACH(_testname)
ENDMACRO(KIO_UNIT_TESTS)
MACRO(KIO_EXECUTABLE_TESTS)
       FOREACH(_testname ${ARGN})
               add_executable(${_testname} ${_testname}.cpp)
               ecm_mark_as_test(${_testname})
               target_link_libraries(${_testname} ${KDE4_KIO_LIBS} Qt5::Test XmlGui KF5::KCoreAddons KF5::KI18n KF5::KIconThemes)
       ENDFOREACH(_testname)
ENDMACRO(KIO_EXECUTABLE_TESTS)

# jobtest seems to trigger a ctest problem; jobtest finishes and ctest waits for ever

KIO_UNIT_TESTS(
  kacltest
)

KIO_EXECUTABLE_TESTS(
  ksycocatest
  speed
  kionetrctest
  kpropsdlgtest
  ksycocaupdatetest
  kdbusservicestartertest
  kfiledialogtest
)


########### kfstest ###############

set(kfstest_SRCS kfstest.cpp kfdtest.cpp)
add_executable(kfstest ${kfstest_SRCS})
ecm_mark_as_test(kfstest)
target_link_libraries(kfstest KF5::KIconThemes ${KDE4_KIO_LIBS} KF5::KCoreAddons KF5::KWidgetsAddons)


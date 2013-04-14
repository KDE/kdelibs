remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include(ECMMarkAsTest)

########### unittests ###############

find_package(Qt5Transitional REQUIRED MODULE COMPONENTS Concurrent)

macro(KSERVICE_UNIT_TESTS)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp kwaitforsignal.cpp)
    add_test(kservice-${_testname} ${_testname})
    target_link_libraries(${_testname}
       kservice
       Qt5::Test
       ${QT_QTXML_LIBRARY}
       ${KConfig_CORE_LIBRARY}
       ki18n
       Qt5::Concurrent
    )
    ecm_mark_as_test(${_testname})
  endforeach()
endmacro(KSERVICE_UNIT_TESTS)

kservice_unit_tests(
 ksycocadicttest
 ksycocathreadtest
 kservicetest
)


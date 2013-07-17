remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include(ECMMarkAsTest)

########### unittests ###############

find_package(Qt5Concurrent 5.2.0 REQUIRED NO_MODULE)

macro(KIOCORE_UNIT_TESTS)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    add_test(kiocore-${_testname} ${_testname})
    target_link_libraries(${_testname}
       KIOCore
       Qt5::Test
       Qt5::Xml
       KI18n
       Qt5::Concurrent
       ${KConfig_CORE_LIBRARY}
    )
    ecm_mark_as_test(${_testname})
  endforeach()
endmacro(KIOCORE_UNIT_TESTS)

kiocore_unit_tests(
 kmountpointtest
)


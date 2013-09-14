remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include(ECMMarkAsTest)

########### unittests ###############

find_package(Qt5Concurrent 5.2.0 REQUIRED NO_MODULE)

macro(KSERVICE_UNIT_TESTS)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp kwaitforsignal.cpp)
    add_test(kservice-${_testname} ${_testname})
    target_link_libraries(${_testname}
       KService
       Qt5::Test
       Qt5::Concurrent
    )
    ecm_mark_as_test(${_testname})
  endforeach()
endmacro(KSERVICE_UNIT_TESTS)


# Build a minimal plugin, equipped with metadata for
# the pluginlocatortest
add_library(fakeplugin MODULE nsaplugin.cpp)
set_target_properties(fakeplugin PROPERTIES PREFIX "")
target_link_libraries(fakeplugin KService KCoreAddons)


kservice_unit_tests(
 kautostarttest
 ksycocadicttest
 ksycocathreadtest
 kservicetest
 pluginlocatortest
)


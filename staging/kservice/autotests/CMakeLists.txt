remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include_directories(
   ${CMAKE_CURRENT_BINARY_DIR}/../src
   ${CMAKE_CURRENT_SOURCE_DIR}/../src/sycoca
   ${CMAKE_CURRENT_SOURCE_DIR}/../src/services
   ${CMAKE_CURRENT_SOURCE_DIR}/../src/plugin
)

include_directories(
  ${kconfig_SOURCE_DIR}/src/core
  ${kconfig_BINARY_DIR}/src/core
)

include(ECMMarkAsTest)

########### unittests ###############

macro(KSERVICE_UNIT_TESTS)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp kwaitforsignal.cpp)
    add_test(kservice-${_testname} ${_testname})
    target_link_libraries(${_testname}
       kservice
       ${QT_QTTEST_LIBRARY}
       ${QT_QTXML_LIBRARY}
       ${QT_QTMAIN_LIBRARY}
       kconfigcore
       ki18n
    )
    #target_link_libraries(${_testname} kservice ${QT_QTCONCURRENT_LIBRARY})
    ecm_mark_as_test(${_testname})
  endforeach()
endmacro(KSERVICE_UNIT_TESTS)

kservice_unit_tests(
 kaboutdatatest
 ksycocadicttest
 ksycocathreadtest
 kservicetest
)


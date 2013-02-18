
set(proxyModelTestSources
  kselectionproxymodeltestsuite.cpp
)


remove_definitions(-DQT_NO_CAST_TO_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)

find_package(Qt5Transitional REQUIRED MODULE COMPONENTS Test Widgets)

add_subdirectory(proxymodeltestsuite)

include(ECMMarkAsTest)

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/../src
   ${CMAKE_CURRENT_BINARY_DIR}/../src
   ${CMAKE_CURRENT_SOURCE_DIR}/proxymodeltestsuite
   ${CMAKE_CURRENT_BINARY_DIR}/proxymodeltestsuite
)

macro(ITEMMODELS_PROXYMODEL_TESTS)
    foreach(_testname ${ARGN})
        add_executable(${_testname} ${_testname}.cpp ${proxyModelTestSources} )
        add_test(itemmodels-${_testname} ${_testname})
        ecm_mark_as_test(${_testname})
        target_link_libraries(${_testname} itemmodels ${QT_QTTEST_LIBRARY} Qt5::Widgets ${QT_QTXML_LIBRARY} ${QT_QTMAIN_LIBRARY} proxymodeltestsuite )
    endforeach(_testname)
endmacro()


itemmodels_proxymodel_tests(
  kdescendantsproxymodeltest
  # BROKEN WITH QT5 kselectionproxymodeltest
  testmodelqueuedconnections
)

if (Qt5Script_FOUND)
    add_subdirectory(proxymodeltestapp)
endif()

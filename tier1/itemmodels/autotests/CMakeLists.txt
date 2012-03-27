
SET(proxyModelTestSources
  kselectionproxymodeltestsuite.cpp
)


remove_definitions(-DQT_NO_CAST_TO_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)

find_package(Qt5Transitional REQUIRED Test Gui)
enable_testing()

add_subdirectory(proxymodeltestsuite)
include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/../src
   ${CMAKE_CURRENT_BINARY_DIR}/../src
   ${CMAKE_CURRENT_SOURCE_DIR}/proxymodeltestsuite
   ${CMAKE_CURRENT_BINARY_DIR}/proxymodeltestsuite
)

MACRO(ITEMMODELS_PROXYMODEL_TESTS)
       FOREACH(_testname ${ARGN})
               add_executable(${_testname} ${_testname}.cpp ${proxyModelTestSources} )
               add_test(itemmodels-${_testname} ${_testname})
               target_link_libraries(${_testname} itemmodels ${QT_QTTEST_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTXML_LIBRARY} proxymodeltestsuite )
       ENDFOREACH(_testname)
ENDMACRO()


ITEMMODELS_PROXYMODEL_TESTS(
  kdescendantsproxymodeltest
  kselectionproxymodeltest
  testmodelqueuedconnections
)

if (Qt5Script_FOUND)
    add_subdirectory(proxymodeltestapp)
endif()

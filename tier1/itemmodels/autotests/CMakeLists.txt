
SET(proxyModelTestSources
  kselectionproxymodeltestsuite.cpp
)

add_subdirectory(proxymodeltestsuite)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src ${CMAKE_CURRENT_SOURCE_DIR}/proxymodeltestsuite)

MACRO(KDEUI_PROXYMODEL_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_add_unit_test(${_testname} TESTNAME itemmodels-${_testname} ${_testname}.cpp ${proxyModelTestSources} )
               target_link_libraries(${_testname} itemmodels ${QT_QTTEST_LIBRARY} ${QT_QTXML_LIBRARY} ${KDEWIN32_LIBRARIES} proxymodeltestsuite )
       ENDFOREACH(_testname)
ENDMACRO(KDEUI_PROXYMODEL_TESTS)


KDEUI_PROXYMODEL_TESTS(
  kdescendantsproxymodeltest
  kselectionproxymodeltest
  testmodelqueuedconnections
)

#add_subdirectory(proxymodeltestapp)


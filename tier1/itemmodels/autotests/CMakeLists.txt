
set(proxyModelTestSources
  kselectionproxymodeltestsuite.cpp
)


remove_definitions(-DQT_NO_CAST_TO_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)

find_package(Qt5 5.2.0 CONFIG REQUIRED Test Widgets)

add_subdirectory(proxymodeltestsuite)

include(ECMMarkAsTest)

macro(ITEMMODELS_PROXYMODEL_TESTS)
    foreach(_testname ${ARGN})
        add_executable(${_testname} ${_testname}.cpp ${proxyModelTestSources} )
        add_test(itemmodels-${_testname} ${_testname})
        ecm_mark_as_test(${_testname})
        target_link_libraries(${_testname} KF5::ItemModels Qt5::Test Qt5::Widgets proxymodeltestsuite )
    endforeach(_testname)
endmacro()


itemmodels_proxymodel_tests(
  kdescendantsproxymodeltest
  klinkitemselectionmodeltest
  kselectionproxymodeltest
  testmodelqueuedconnections
)

if (Qt5Script_FOUND)
    add_subdirectory(proxymodeltestapp)
endif()

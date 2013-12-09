remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_TO_ASCII)

add_subdirectory(pluginlocator)

include(ECMMarkAsTest)

find_package(Qt5 5.2.0 CONFIG REQUIRED Test)

macro(kservice_executable_tests)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    target_link_libraries(${_testname} KF5::Service)
    ecm_mark_as_test(${_testname})
  endforeach(_testname)
endmacro()

kservice_executable_tests(
  startserviceby
  kdbusservicestartertest
  ksycocatest
)

######### kmimeassociationstest ########

set(kmimeassociationstest_SRCS kmimeassociationstest.cpp ../src/kbuildsycoca/kmimeassociations.cpp)

add_executable(kmimeassociationstest ${kmimeassociationstest_SRCS})
ecm_mark_as_test(kmimeassociationstest)

target_link_libraries(kmimeassociationstest KF5::Service Qt5::Test)
target_include_directories(kmimeassociationstest PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../src/kbuildsycoca/")

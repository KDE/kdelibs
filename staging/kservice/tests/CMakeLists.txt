remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_TO_ASCII)

add_subdirectory(pluginlocator)

include(ECMMarkAsTest)

find_package(Qt5 5.2.0 REQUIRED NO_MODULE COMPONENTS Test)

macro(kservice_executable_tests)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    target_link_libraries(${_testname} KService)
    ecm_mark_as_test(${_testname})
  endforeach(_testname)
endmacro()

kservice_executable_tests(
  startserviceby
)

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

######### kmimeassociationstest ########

set(kmimeassociationstest_SRCS kmimeassociationstest.cpp ../kbuildsycoca/kmimeassociations.cpp)

add_executable(kmimeassociationstest ${kmimeassociationstest_SRCS})
ecm_mark_as_test(kmimeassociationstest)

target_link_libraries(kmimeassociationstest KConfigCore KService Qt5::Test)
target_include_directories(kmimeassociationstest PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../kbuildsycoca/")
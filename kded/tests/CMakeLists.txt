
set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

######### kmimeassociationstest ########

set(kmimeassociationstest_SRCS kmimeassociationstest.cpp ../kmimeassociations.cpp)

kde4_add_unit_test(kmimeassociationstest ${kmimeassociationstest_SRCS})

target_link_libraries(kmimeassociationstest KConfigCore KF5::KService Qt5::Test)
target_include_directories(kmimeassociationstest PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/..")

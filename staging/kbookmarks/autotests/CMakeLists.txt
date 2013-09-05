# TODO
remove_definitions(-DQT_NO_CAST_FROM_ASCII)

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include(ECMMarkAsTest)

add_executable(kbookmarktest kbookmarktest.cpp)
ecm_mark_as_test(${kbookmarktest})
target_link_libraries(kbookmarktest KBookmarks Qt5::Test)


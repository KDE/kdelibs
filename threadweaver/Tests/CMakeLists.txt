
include_directories(${QT_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR}/Weaver/)

include_directories( ../Weaver/ )

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

########### next target ###############

set(JobTests_SRCS JobTests.cpp AppendCharacterJob.cpp)


kde4_add_unit_test(JobTests ${JobTests_SRCS})

target_link_libraries(JobTests ${QT_QTTEST_LIBRARY} threadweaver)

########### next target ###############

set(QueueTests_SRCS QueueTests.cpp AppendCharacterJob.cpp)


kde4_add_unit_test(QueueTests ${QueueTests_SRCS})

target_link_libraries(QueueTests ${QT_QTTEST_LIBRARY} threadweaver)

########### next target ###############

set(DeleteTest_SRCS DeleteTest.cpp AppendCharacterJob.cpp)


kde4_add_unit_test(DeleteTest ${DeleteTest_SRCS})

target_link_libraries(DeleteTest ${QT_QTTEST_LIBRARY} threadweaver)


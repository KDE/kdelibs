
include_directories(${QT_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR}/Weaver/)

include_directories( ../Weaver/ )

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

########### next target ###############

## TODO: re-enable and fix this test!

#set(JobTests_SRCS JobTests.cpp AppendCharacterJob.cpp)

#kde4_add_unit_test(ThreadWeaverJobTests ${JobTests_SRCS})

#target_link_libraries(ThreadWeaverJobTests ${QT_QTTEST_LIBRARY} threadweaver)

########### next target ###############

set(QueueTests_SRCS QueueTests.cpp AppendCharacterJob.cpp)


kde4_add_unit_test(ThreadWeaverQueueTests ${QueueTests_SRCS})

target_link_libraries(ThreadWeaverQueueTests ${QT_QTTEST_LIBRARY} ${QT_QTCORE_LIBRARY} threadweaver)

########### next target ###############

set(DeleteTest_SRCS DeleteTest.cpp AppendCharacterJob.cpp)


kde4_add_unit_test(ThreadWeaverDeleteTest ${DeleteTest_SRCS})

target_link_libraries(ThreadWeaverDeleteTest ${QT_QTTEST_LIBRARY} ${QT_QTCORE_LIBRARY} threadweaver)


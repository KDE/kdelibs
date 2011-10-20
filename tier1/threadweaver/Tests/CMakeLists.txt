
include_directories(
     ${CMAKE_CURRENT_SOURCE_DIR}/../Weaver/
     ${CMAKE_CURRENT_BINARY_DIR}/../Weaver/
     )

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

########### next target ###############

set(JobTests_SRCS JobTests.cpp AppendCharacterJob.cpp)

add_executable(ThreadWeaverJobTests ${JobTests_SRCS})
add_test(ThreadWeaverJobTests ThreadWeaverJobTests)

target_link_libraries(ThreadWeaverJobTests ${QT_QTTEST_LIBRARY} threadweaver)

########### next target ###############

set(QueueTests_SRCS QueueTests.cpp AppendCharacterJob.cpp)

add_executable(ThreadWeaverQueueTests ${QueueTests_SRCS})
add_test(ThreadWeaverQueueTests ThreadWeaverQueueTests)

target_link_libraries(ThreadWeaverQueueTests ${QT_QTTEST_LIBRARY} ${QT_QTCORE_LIBRARY} threadweaver)

########### next target ###############

set(DeleteTest_SRCS DeleteTest.cpp AppendCharacterJob.cpp)

add_executable(ThreadWeaverDeleteTest ${DeleteTest_SRCS})
add_test(ThreadWeaverDeleteTest ThreadWeaverDeleteTest)

target_link_libraries(ThreadWeaverDeleteTest ${QT_QTTEST_LIBRARY} ${QT_QTCORE_LIBRARY} threadweaver)


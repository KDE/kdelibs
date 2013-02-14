
remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include(ECMMarkAsTest)

########### next target ###############

set(JobTests_SRCS JobTests.cpp AppendCharacterJob.cpp AppendCharacterAndVerifyJob.cpp)

add_executable(ThreadWeaverJobTests ${JobTests_SRCS})
add_test(ThreadWeaverJobTests ThreadWeaverJobTests)
ecm_mark_as_test(ThreadWeaverJobTests)

target_link_libraries(ThreadWeaverJobTests ${QT_QTTEST_LIBRARY} ThreadWeaver)

########### next target ###############

set(QueueTests_SRCS QueueTests.cpp AppendCharacterJob.cpp)

add_executable(ThreadWeaverQueueTests ${QueueTests_SRCS})
add_test(ThreadWeaverQueueTests ThreadWeaverQueueTests)
ecm_mark_as_test(ThreadWeaverQueueTests)

target_link_libraries(ThreadWeaverQueueTests ${QT_QTTEST_LIBRARY} ThreadWeaver)

########### next target ###############

set(DeleteTest_SRCS DeleteTest.cpp AppendCharacterJob.cpp)

add_executable(ThreadWeaverDeleteTest ${DeleteTest_SRCS})
add_test(ThreadWeaverDeleteTest ThreadWeaverDeleteTest)
ecm_mark_as_test(ThreadWeaverDeleteTest)

target_link_libraries(ThreadWeaverDeleteTest ${QT_QTTEST_LIBRARY} ThreadWeaver)

########### next target ###############

set(QueueBenchmarks_SRCS QueueBenchmarks.cpp)

add_executable(ThreadWeaverQueueBenchmarks ${QueueBenchmarks_SRCS})
add_test(ThreadWeaverQueueBenchmarks ThreadWeaverQueueBenchmarks)
ecm_mark_as_test(ThreadWeaverQueueBenchmarks)

target_link_libraries(ThreadWeaverQueueBenchmarks ${QT_QTTEST_LIBRARY} ${QT_QTMAIN_LIBRARY} threadweaver)

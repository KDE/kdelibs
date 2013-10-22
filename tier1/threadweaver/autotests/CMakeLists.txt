remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include(ECMMarkAsTest)

########### next target ###############

set(JobTests_SRCS JobTests.cpp AppendCharacterAndVerifyJob.cpp)

add_executable(ThreadWeaverJobTests ${JobTests_SRCS})
add_test(ThreadWeaverJobTests ThreadWeaverJobTests)
ecm_mark_as_test(ThreadWeaverJobTests)

target_link_libraries(ThreadWeaverJobTests Qt5::Test ThreadWeaver)

########### next target ###############

set(QueueTests_SRCS QueueTests.cpp)

add_executable(ThreadWeaverQueueTests ${QueueTests_SRCS})
add_test(ThreadWeaverQueueTests ThreadWeaverQueueTests)
ecm_mark_as_test(ThreadWeaverQueueTests)

target_link_libraries(ThreadWeaverQueueTests Qt5::Test ThreadWeaver)

########### next target ###############

set(DeleteTest_SRCS DeleteTest.cpp)

add_executable(ThreadWeaverDeleteTest ${DeleteTest_SRCS})
add_test(ThreadWeaverDeleteTest ThreadWeaverDeleteTest)
ecm_mark_as_test(ThreadWeaverDeleteTest)

target_link_libraries(ThreadWeaverDeleteTest Qt5::Test ThreadWeaver)

########### next target ###############

set(LifecycleTest_SRCS LifecycleTests.cpp)

add_executable(ThreadWeaverLifecycleTest ${LifecycleTest_SRCS})
add_test(ThreadWeaverLifecycleTest ThreadWeaverLifecycleTest)
ecm_mark_as_test(ThreadWeaverLifecycleTest)

target_link_libraries(ThreadWeaverLifecycleTest Qt5::Test ThreadWeaver)

########### next target ###############

set(ShutdownOnQApplicationQuitTest_SRCS ShutdownOnQApplicationQuitTests.cpp)

add_executable(ThreadWeaverShutdownOnQApplicationQuitTest ${ShutdownOnQApplicationQuitTest_SRCS})
add_test(ThreadWeaverShutdownOnQApplicationQuitTest ThreadWeaverShutdownOnQApplicationQuitTest)
ecm_mark_as_test(ThreadWeaverShutdownOnQApplicationQuitTest)

target_link_libraries(ThreadWeaverShutdownOnQApplicationQuitTest Qt5::Test ThreadWeaver)

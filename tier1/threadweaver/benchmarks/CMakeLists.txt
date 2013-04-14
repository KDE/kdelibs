include_directories(
     ${CMAKE_CURRENT_SOURCE_DIR}/../src/Weaver/
     ${CMAKE_CURRENT_BINARY_DIR}/../src/Weaver/
)

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

########### next target ###############

set(QueueBenchmarks_SRCS QueueBenchmarks.cpp)

add_executable(ThreadWeaverQueueBenchmarks ${QueueBenchmarks_SRCS})
# add_test(ThreadWeaverQueueBenchmarks ThreadWeaverQueueBenchmarks)

target_link_libraries(ThreadWeaverQueueBenchmarks Qt5::Test ThreadWeaver)

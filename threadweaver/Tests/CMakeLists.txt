
# Tests don't need to go into toplevel/bin, they are fine in the current dir.
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(${QT_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR}/Weaver/)


########### next target ###############

set(JobTests_SRCS JobTests.cpp AppendCharacterJob.cpp)

QT4_AUTOMOC(${JobTests_SRCS})

add_executable(JobTests ${JobTests_SRCS})
ADD_TEST(JobTests JobTests)

target_link_libraries(JobTests ${QT_QTTEST_LIBRARY} ThreadWeaver)

########### next target ###############

set(QueueTests_SRCS QueueTests.cpp AppendCharacterJob.cpp)

QT4_AUTOMOC(${QueueTests_SRCS})

add_executable(QueueTests ${QueueTests_SRCS})
ADD_TEST(QueueTests QueueTests)

target_link_libraries(QueueTests ${QT_QTTEST_LIBRARY} ThreadWeaver)


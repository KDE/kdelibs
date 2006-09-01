
if(KDE4_BUILD_TESTS)

# Tests don't need to go into toplevel/bin, they are fine in the current dir.
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(${QT_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR}/Weaver/)


########### next target ###############

set(AppendCharacterJob_SRCS JobTests.cpp AppendCharacterJob.cpp)

kde4_automoc(${AppendCharacterJob_SRCS})

kde4_add_executable(AppendCharacterJob RUN_UNINSTALLED ${AppendCharacterJob_SRCS})

target_link_libraries(AppendCharacterJob ${QT_QTTEST_LIBRARY} ThreadWeaver)

########### next target ###############

set(QueueTests_SRCS QueueTests.cpp AppendCharacterJob.cpp)

kde4_automoc(${QueueTests_SRCS})

kde4_add_executable(QueueTests RUN_UNINSTALLED ${QueueTests_SRCS})

target_link_libraries(QueueTests ${QT_QTTEST_LIBRARY} ThreadWeaver)




endif(KDE4_BUILD_TESTS)

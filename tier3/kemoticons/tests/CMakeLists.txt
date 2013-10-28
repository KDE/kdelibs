set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

add_executable(kemoticonstest main.cpp)

target_link_libraries(kemoticonstest KEmoticons)

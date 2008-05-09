set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

include_directories(${CMAKE_SOURCE_DIR} ../ ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDES})

kde4_add_executable(kemoticonstest main.cpp)

target_link_libraries(kemoticonstest ${KDE4_KDEUI_LIBS} kemoticons)

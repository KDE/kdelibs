set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

project(goyatest)

include_directories(${KDE4_INCLUDES} ${CMAKE_SOURCE_DIR} ../)

kde4_add_executable(kemoticonstest main.cpp)

target_link_libraries(kemoticonstest ${KDE4_KDEUI_LIBS} kemoticons)

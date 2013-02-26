set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/../src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/core
    ${CMAKE_SOURCE_DIR}/kdeui/widgets
    ${KDE4_KDEUI_INCLUDES}
    ${CMAKE_BINARY_DIR}/staging/kwidgets/src
    ${CMAKE_SOURCE_DIR}/staging/kwidgets/src/utils
)

kde4_add_executable(kemoticonstest main.cpp)
target_link_libraries(kemoticonstest ${KDE4_KDEUI_LIBS} ${KCoreAddons_LIBRARIES} kemoticons)

project(kemoticons)

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDES})

set(kemoticons_LIB_SRCS kemoticons.cpp kemoticonstheme.cpp)

kde4_add_library(kemoticons SHARED ${emoticons_LIB_SRCS})

target_link_libraries(kemoticons ${KDE4_KDEUI_LIBS} )

install(TARGETS kemoticons DESTINATION ${LIB_INSTALL_DIR})

install(FILES emoticonsTheme.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})
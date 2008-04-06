project(emoticons)

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDES})

set(emoticons_LIB_SRCS emoticons.cpp)

kde4_add_library(emoticons SHARED ${emoticons_LIB_SRCS})

target_link_libraries(emoticons ${KDE4_KDEUI_LIBS} )

install(TARGETS emoticons DESTINATION ${LIB_INSTALL_DIR})


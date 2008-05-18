project(kemoticons)

find_package(KDE4 REQUIRED)

add_subdirectory(providers)
add_subdirectory(tests)

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDES})

set(kemoticons_LIB_SRCS kemoticons.cpp kemoticonstheme.cpp kemoticonsprovider.cpp)

kde4_add_library(kemoticons SHARED ${kemoticons_LIB_SRCS})

target_link_libraries(kemoticons ${KDE4_KIO_LIBS})

set_target_properties(kemoticons PROPERTIES VERSION 1.0.0 SOVERSION 1)

install(TARGETS kemoticons DESTINATION ${LIB_INSTALL_DIR})

install(FILES kemoticonsTheme.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})

install(FILES kemoticons.h kemoticonstheme.h kemoticonsprovider.h kemoticons_export.h DESTINATION ${INCLUDE_INSTALL_DIR})

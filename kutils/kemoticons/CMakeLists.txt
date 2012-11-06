project(kemoticons)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}
                     ${CMAKE_CURRENT_BINARY_DIR}
)

add_subdirectory(providers)
add_subdirectory(tests)

set(kemoticons_LIB_SRCS
    kemoticons.cpp
    kemoticonstheme.cpp
    kemoticonsprovider.cpp
)

kde4_add_library(kemoticons ${LIBRARY_TYPE} ${kemoticons_LIB_SRCS})
generate_export_header(kemoticons)
target_link_libraries(kemoticons ${QtMimeTypes_LIBRARY})
target_link_libraries(kemoticons LINK_INTERFACE_LIBRARIES ${QT_QTGUI_LIBRARY})
set_target_properties(kemoticons PROPERTIES
                      VERSION ${GENERIC_LIB_VERSION}
                      SOVERSION ${GENERIC_LIB_SOVERSION})

install(FILES kemoticonsTheme.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/kemoticons_export.h
    kemoticons.h
    kemoticonstheme.h
    kemoticonsprovider.h
    DESTINATION ${INCLUDE_INSTALL_DIR}
    COMPONENT Devel
)

install(TARGETS kemoticons EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

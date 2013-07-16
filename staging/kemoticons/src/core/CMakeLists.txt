############# kemoticons library ###################

set(kemoticons_LIB_SRCS
    kemoticons.cpp
    kemoticonstheme.cpp
    kemoticonsprovider.cpp
)

add_library(KEmoticons ${LIBRARY_TYPE} ${kemoticons_LIB_SRCS})
generate_export_header(KEmoticons)
target_link_libraries(KEmoticons kio ${KCoreAddons_LIBRARIES} ${KArchive_LIBRARIES} ${KConfig_CORE_LIBRARY} KService KWidgets)
target_link_libraries(KEmoticons LINK_INTERFACE_LIBRARIES Qt5::Widgets)
set_target_properties(KEmoticons PROPERTIES
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

install(TARGETS KEmoticons EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

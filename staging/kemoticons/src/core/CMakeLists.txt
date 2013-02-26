############# kemoticons library ###################
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${KDE4_KIO_INCLUDES}
    ${KCoreAddons_INCLUDE_DIRS}
    ${KArchive_INCLUDE_DIRS}
    ${CMAKE_BINARY_DIR}/tier2/kconfig/src/core # kconfigcore_export.h
    ${CMAKE_SOURCE_DIR}/tier2/kconfig/src/core # ksharedconfig.h
    ${CMAKE_BINARY_DIR}/staging/kwidgets/src
)

set(kemoticons_LIB_SRCS
    kemoticons.cpp
    kemoticonstheme.cpp
    kemoticonsprovider.cpp
)

kde4_add_library(kemoticons ${LIBRARY_TYPE} ${kemoticons_LIB_SRCS})
generate_export_header(kemoticons)
target_link_libraries(kemoticons kio ${KCoreAddons_LIBRARIES} ${KArchive_LIBRARIES} ${KConfig_CORE_LIBRARY} kservice kwidgets)
target_link_libraries(kemoticons LINK_INTERFACE_LIBRARIES Qt5::Widgets)
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

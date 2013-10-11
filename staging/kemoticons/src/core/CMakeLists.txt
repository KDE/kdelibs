############# kemoticons library ###################

set(kemoticons_LIB_SRCS
    kemoticons.cpp
    kemoticonstheme.cpp
    kemoticonsprovider.cpp
)

add_library(KEmoticons ${kemoticons_LIB_SRCS})
generate_export_header(KEmoticons)
add_library(KF5::KEmoticons ALIAS KEmoticons)

target_link_libraries(KEmoticons
LINK_PUBLIC
    Qt5::Widgets # for QPixmap
    KService     # for KService::Ptr, KServiceTypeTrader
LINK_PRIVATE
    KArchive    # for KTar, KZip
)

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

install(TARGETS KEmoticons EXPORT KEmoticonsTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

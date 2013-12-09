############# kemoticons library ###################

set(kemoticons_LIB_SRCS
    kemoticons.cpp
    kemoticonstheme.cpp
    kemoticonsprovider.cpp
)

add_library(KF5Emoticons ${kemoticons_LIB_SRCS})
generate_export_header(KF5Emoticons BASE_NAME KEmoticons)
add_library(KF5::Emoticons ALIAS KF5Emoticons)

target_link_libraries(KF5Emoticons
PUBLIC
    Qt5::Widgets # for QPixmap
    KF5::Service     # for KService::Ptr, KServiceTypeTrader
PRIVATE
    KF5::Archive    # for KTar, KZip
)

set_target_properties(KF5Emoticons PROPERTIES
                      VERSION ${KEMOTICONS_VERSION_STRING}
                      SOVERSION ${KEMOTICONS_SOVERSION}
                      EXPORT_NAME Emoticons
)

install(FILES kemoticonsTheme.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/kemoticons_export.h
    kemoticons.h
    kemoticonstheme.h
    kemoticonsprovider.h
    DESTINATION ${INCLUDE_INSTALL_DIR}
    COMPONENT Devel
)

install(TARGETS KF5Emoticons EXPORT KF5EmoticonsTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

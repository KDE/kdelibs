project(kemoticons)

include_directories( 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_BINARY_DIR}/libinqt5
    ${CMAKE_SOURCE_DIR}/libinqt5/src/libqtmimetypes/src/mimetypes
    ${CMAKE_BINARY_DIR}/kdecore
    ${CMAKE_SOURCE_DIR}/kdecore/io
    ${CMAKE_SOURCE_DIR}/kdecore/kernel
    ${CMAKE_SOURCE_DIR}/kio
    ${CMAKE_BINARY_DIR}/tier1/kcoreaddons/src
    ${CMAKE_SOURCE_DIR}/tier1/kcoreaddons/src/jobs # kjob.h
    ${CMAKE_SOURCE_DIR}/tier1/kcoreaddons/src/io # kdirwatch.h
    ${CMAKE_BINARY_DIR}/tier1/karchive/src
    ${CMAKE_SOURCE_DIR}/tier1/karchive/src
    ${CMAKE_BINARY_DIR}/tier2/kconfig/src/core # kconfigcore_export.h
    ${CMAKE_SOURCE_DIR}/tier2/kconfig/src/core # ksharedconfig.h
    ${CMAKE_BINARY_DIR}/staging/kservice/src # kservice_export.h
    ${CMAKE_SOURCE_DIR}/staging/kservice/src/services # kservicetrader.h
    ${CMAKE_SOURCE_DIR}/staging/kservice/src/plugin # kpluginfactory.h
    ${CMAKE_SOURCE_DIR}/staging/kservice/src/sycoca # ksycocaentry.h
    ${CMAKE_BINARY_DIR}/staging/kwidgets/src
    ${CMAKE_SOURCE_DIR}/staging/kwidgets/src/icons # kiconloader.h
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
target_link_libraries(kemoticons ${QtMimeTypes_LIBRARY} kdecore kio kcoreaddons karchive kconfigcore kservice kwidgets)
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

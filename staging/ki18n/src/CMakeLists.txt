set(libki18n_SRCS
    klocalizedstring.cpp
    kcatalog.cpp
    ktranscriptinterface.cpp
    kuitformats.cpp
    kuitsemantics.cpp
    common_helpers.cpp
)

add_library(ki18n ${libki18n_SRCS})
generate_export_header(ki18n)

target_link_libraries(ki18n LINK_PUBLIC  ${QT_QTCORE_LIBRARY}
                            LINK_PRIVATE ${QtMimeTypes_LIBRARY} ${QT_QTXML_LIBRARY})

set_target_properties(ki18n PROPERTIES VERSION   ${KI18N_VERSION_STRING}
                                       SOVERSION ${KI18N_SOVERSION}
)

install(TARGETS ki18n EXPORT ki18nTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    klocalizedstring.h
    ktranscriptinterface.h
    ${CMAKE_CURRENT_BINARY_DIR}/ki18n_export.h
    DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)


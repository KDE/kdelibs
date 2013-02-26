
set(libkdbusaddons_SRCS
    kdbusconnectionpool.cpp
    kdbusinterprocesslock.cpp
    kdbusservice.cpp
)

add_library(KDBusAddons ${libkdbusaddons_SRCS})
generate_export_header(KDBusAddons)

target_link_libraries(KDBusAddons ${QT_QTDBUS_LIBRARY})

set_target_properties(KDBusAddons PROPERTIES VERSION   ${KDBUSADDONS_VERSION_STRING}
                                             SOVERSION ${KDBUSADDONS_SOVERSION}
)

install(TARGETS KDBusAddons EXPORT KDBusAddonsTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    kdbusconnectionpool.h
    kdbusinterprocesslock.h
    kdbusservice.h
    ${CMAKE_CURRENT_BINARY_DIR}/kdbusaddons_export.h
    DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)


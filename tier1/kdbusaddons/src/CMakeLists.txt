
set(libkdbusaddons_SRCS
    kdbusconnectionpool.cpp
    kdbusinterprocesslock.cpp
    kdbusservice.cpp
)

add_library(kdbusaddons ${libkdbusaddons_SRCS})
generate_export_header(kdbusaddons)

target_link_libraries(kdbusaddons ${QT_QTDBUS_LIBRARY})

set_target_properties(kdbusaddons PROPERTIES VERSION   ${KDBUSADDONS_VERSION_STRING}
                                             SOVERSION ${KDBUSADDONS_SOVERSION}
)

install(TARGETS kdbusaddons EXPORT kdbusaddonsTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    kdbusconnectionpool.h
    kdbusinterprocesslock.h
    kdbusservice.h
    ${CMAKE_CURRENT_BINARY_DIR}/kdbusaddons_export.h
    DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)


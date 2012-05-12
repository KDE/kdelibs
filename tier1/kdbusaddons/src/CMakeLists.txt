
set(libkdbusaddons_SRCS
    kdbusconnectionpool.cpp
    kdbusinterprocesslock.cpp
    kdbusservice.cpp
)

add_library(kdbusaddons ${libkdbusaddons_SRCS})
generate_export_header(kdbusaddons)

target_link_libraries(kdbusaddons ${QT_QTCORE_LIBRARY} ${QT_QTDBUS_LIBRARY})

set_target_properties(kdbusaddons PROPERTIES VERSION   ${ECM_VERSION_STRING}
                                             SOVERSION ${ECM_SOVERSION}
)

install(TARGETS kdbusaddons EXPORT kdelibsLibraryTargets ${ECM_TARGET_DEFAULT_ARGS})

install(FILES
    kdbusconnectionpool.h
    kdbusinterprocesslock.h
    kdbusservice.h
    ${CMAKE_CURRENT_BINARY_DIR}/kdbusaddons_export.h
    DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)


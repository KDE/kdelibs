
set(libkdbusaddons_SRCS
    kdbusconnectionpool.cpp
    kdbusinterprocesslock.cpp
    kdbusservice.cpp
    kdedmodule.cpp
)

qt5_add_dbus_adaptor(libkdbusaddons_SRCS org.freedesktop.Application.xml kdbusservice.h KDBusService kdbusservice_adaptor KDBusServiceAdaptor)

add_library(KDBusAddons ${libkdbusaddons_SRCS})
generate_export_header(KDBusAddons)



target_link_libraries(KDBusAddons Qt5::DBus)

set_target_properties(KDBusAddons PROPERTIES VERSION   ${KDBUSADDONS_VERSION_STRING}
                                             SOVERSION ${KDBUSADDONS_SOVERSION}
)

install(TARGETS KDBusAddons EXPORT KDBusAddonsTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    kdbusconnectionpool.h
    kdbusinterprocesslock.h
    kdbusservice.h
    kdedmodule.h
    ${CMAKE_CURRENT_BINARY_DIR}/kdbusaddons_export.h
    DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)


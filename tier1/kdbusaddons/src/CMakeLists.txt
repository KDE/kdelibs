
set(libkdbusaddons_SRCS
    kdbusconnectionpool.cpp
    kdbusinterprocesslock.cpp
    kdbusservice.cpp
    kdedmodule.cpp
    kdeinit_interface.cpp
)

qt5_add_dbus_adaptor(libkdbusaddons_SRCS
   org.freedesktop.Application.xml
   kdbusservice.h
   KDBusService
   kdbusservice_adaptor
   KDBusServiceAdaptor)
qt5_add_dbus_adaptor(libkdbusaddons_SRCS
   org.kde.KDBusService.xml
   kdbusservice.h
   KDBusService
   kdbusserviceextensions_adaptor
   KDBusServiceExtensionsAdaptor)

add_library(KF5DBusAddons ${libkdbusaddons_SRCS})
generate_export_header(KF5DBusAddons BASE_NAME KDBusAddons)
add_library(KF5::DBusAddons ALIAS KF5DBusAddons)



target_link_libraries(KF5DBusAddons PUBLIC Qt5::DBus)

set_target_properties(KF5DBusAddons PROPERTIES VERSION   ${KDBUSADDONS_VERSION_STRING}
                                               SOVERSION ${KDBUSADDONS_SOVERSION}
                                               EXPORT_NAME DBusAddons
)

install(TARGETS KF5DBusAddons EXPORT KF5DBusAddonsTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    kdbusconnectionpool.h
    kdbusinterprocesslock.h
    kdbusservice.h
    kdedmodule.h
    kdeinit_interface.h
    ${CMAKE_CURRENT_BINARY_DIR}/kdbusaddons_export.h
    DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)


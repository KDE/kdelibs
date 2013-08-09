
set(solidextensionplugin_SRCS
    ${solid_common_SRCS}
    solidextensionplugin.cpp
    devices.cpp
    )

# find_package(Qt5 5.2.0 REQUIRED NO_MODULE COMPONENTS Core Widgets DBus Declarative Quick Qml)

add_library(solidextensionplugin SHARED ${solidextensionplugin_SRCS})

qt5_use_modules(solidextensionplugin Core Widgets Declarative DBus Quick Qml)

target_link_libraries(
    solidextensionplugin
    Solid
    Qt5::Core
    Qt5::Declarative
)

install(TARGETS solidextensionplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/solid)
install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/solid)



set(solidextensionplugin_SRCS
    ${solid_common_SRCS}
    solidextensionplugin.cpp
    devices.cpp
    )

add_library(solidextensionplugin SHARED ${solidextensionplugin_SRCS})

find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Qml)

target_link_libraries(
    solidextensionplugin
    Solid
    Qt5::Core
    Qt5::Qml
)

install(TARGETS solidextensionplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/solid)
install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/solid)


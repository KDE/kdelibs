
add_subdirectory(tests)

include_directories (
    ${KDE4_INCLUDES}
    ${QCA2_INCLUDE_DIR} 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

INSTALL(FILES 
    org.freedesktop.Secret.Collection.xml
    org.freedesktop.Secret.Item.xml
    org.freedesktop.Secret.Prompt.xml
    org.freedesktop.Secret.Service.xml
    org.freedesktop.Secret.Session.xml
    DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})


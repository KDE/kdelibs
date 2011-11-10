
add_subdirectory(tests)

INSTALL(FILES 
    org.freedesktop.Secret.Collection.xml
    org.freedesktop.Secret.Item.xml
    org.freedesktop.Secret.Prompt.xml
    org.freedesktop.Secret.Service.xml
    org.freedesktop.Secret.Session.xml
    DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})


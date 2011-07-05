PROJECT( ksecretsserviceclient )

add_subdirectory(tests)

include_directories (
    ${KDE4_INCLUDES}
    ${QCA2_INCLUDE_DIR} 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

set (client_lib_SRCS
    ksecretsserviceitem.cpp
    ksecretsserviceitemjobs.cpp
    ksecretsservicesecret.cpp
    ksecretsservicecollection.cpp
    ksecretscollectionjobs.cpp
    dbusbackend.cpp
    ../daemon/frontend/secret/adaptors/dbustypes.cpp
)



kss_add_dbus_interface(client_lib_SRCS org.freedesktop.Secret.Collection.xml collection_interface)
kss_add_dbus_interface(client_lib_SRCS org.freedesktop.Secret.Item.xml item_interface )
kss_add_dbus_interface(client_lib_SRCS org.freedesktop.Secret.Prompt.xml prompt_interface )
kss_add_dbus_interface(client_lib_SRCS org.freedesktop.Secret.Service.xml service_interface )
kss_add_dbus_interface(client_lib_SRCS org.freedesktop.Secret.Session.xml session_interface )

kde4_add_library(ksecretsserviceclient ${client_lib_SRCS})

TARGET_LINK_LIBRARIES (ksecretsserviceclient 
    ksecretsservicelib
    ${KDE4_KDECORE_LIBS} 
    ${QCA2_LIBRARIES}
)

INSTALL(FILES 
    ${ksecretsserviceclient_xml} 
    DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})

INSTALL(FILES
    ksecretsservicecollection.h
    ksecretsserviceitem.h
    ksecretsservicesecret.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/ksecretsservice COMPONENT Devel 
)

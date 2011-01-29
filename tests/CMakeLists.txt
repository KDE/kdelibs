SET (EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

INCLUDE_DIRECTORIES (
    ${KDE4_INCLUDES}
    ${QCA2_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/daemon/frontend/secret
)

set( ksecretservice_client_test_SRC
   ksecretservice-test.cpp
)

kss_add_dbus_interface( ksecretservice_client_test_SRC ../org.freedesktop.Secret.Service.xml service_interface )
kss_add_dbus_interface( ksecretservice_client_test_SRC ../org.freedesktop.Secret.Session.xml session_interface )
kss_add_dbus_interface( ksecretservice_client_test_SRC ../org.freedesktop.Secret.Collection.xml collection_interface )
kss_add_dbus_interface( ksecretservice_client_test_SRC ../org.freedesktop.Secret.Item.xml item_interface )
kss_add_dbus_interface( ksecretservice_client_test_SRC ../org.freedesktop.Secret.Prompt.xml prompt_interface )

KDE4_ADD_EXECUTABLE( ksecretservice_client_test ${ksecretservice_client_test_SRC} )

TARGET_LINK_LIBRARIES (ksecretservice_client_test
    ksecretserviceclient
   ${QT_QTTEST_LIBRARIES}
)

ADD_TEST( KSecretServiceTest ksecretservice_client_test )

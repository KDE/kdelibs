SET (EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

INCLUDE_DIRECTORIES (
    ${KDE4_INCLUDES}
    ${QCA2_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

set( ksecretsservice_client_test_SRC ksecretsservice-test.cpp
)

kss_add_dbus_interface( ksecretservice_client_test_SRC ../org.freedesktop.Secret.Service.xml service_interface )
kss_add_dbus_interface( ksecretservice_client_test_SRC ../org.freedesktop.Secret.Session.xml session_interface )
kss_add_dbus_interface( ksecretservice_client_test_SRC ../org.freedesktop.Secret.Collection.xml collection_interface )
kss_add_dbus_interface( ksecretservice_client_test_SRC ../org.freedesktop.Secret.Item.xml item_interface )
kss_add_dbus_interface( ksecretservice_client_test_SRC ../org.freedesktop.Secret.Prompt.xml prompt_interface )

KDE4_ADD_EXECUTABLE( ksecretservice_client_test ${ksecretsservice_client_test_SRC} )

TARGET_LINK_LIBRARIES (ksecretsservice_client_test
    ksecretsserviceclient
    ${QT_QTTEST_LIBRARIES}
)

ADD_TEST( KSecrestServiceTest ksecretsservice_client_test )

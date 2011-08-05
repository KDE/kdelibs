PROJECT( ksecretsservice_client_test )

SET (EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

INCLUDE_DIRECTORIES (
    ${KDE4_INCLUDES}
    ${QCA2_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

set( ksecretsservice_client_test_SRC 
    ksecretsservice-test.cpp
)

KDE4_ADD_EXECUTABLE( ksecretsservice_client_test ${ksecretsservice_client_test_SRC} )

TARGET_LINK_LIBRARIES (ksecretsservice_client_test
    ksecretsserviceclient
    ${QT_QTTEST_LIBRARIES}
)

ADD_TEST( KSecretsServiceClientAPITest ksecretsservice_client_test )

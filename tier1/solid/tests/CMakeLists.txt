set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

########### solidnettestdbusservice ###############

#set(solidnettestdbusservice_SRCS
#   solidnettestdbusservice.cpp )


#QT4_ADD_DBUS_ADAPTOR(solidnettestdbusservice_SRCS ../src/solid/org.kde.Solid.Networking.Client.xml
#                         solidnettestdbusservice.h TestNetworkingService)

#kde4_add_executable(solidnettestdbusservice TEST ${solidnettestdbusservice_SRCS})

#target_link_libraries(solidnettestdbusservice ${QT_QTCORE_LIBRARY} ${QT_QTDBUS_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTTEST_LIBRARY})

########### networkingclient ###############

include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})

set(networkingclient_SRCS networkingclient.cpp)

set(CMAKE_AUTOMOC ON)

qt4_wrap_ui(networkingclient_SRCS networkingclientview.ui)

qt4_add_dbus_interface(networkingclient_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/../src/solid/org.kde.Solid.Networking.Client.xml clientinterface)

add_executable(networkingclient ${networkingclient_SRCS})

target_link_libraries(networkingclient solid_static)
########### install files ###############


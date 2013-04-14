include(ECMMarkAsTest)

########### solidnettestdbusservice ###############

#set(solidnettestdbusservice_SRCS
#   solidnettestdbusservice.cpp )


#QT4_ADD_DBUS_ADAPTOR(solidnettestdbusservice_SRCS ../src/solid/org.kde.Solid.Networking.Client.xml
#                         solidnettestdbusservice.h TestNetworkingService)

#kde4_add_executable(solidnettestdbusservice TEST ${solidnettestdbusservice_SRCS})

#target_link_libraries(solidnettestdbusservice Qt5::DBus ${QT_QTXML_LIBRARY} Qt5::Test)

########### networkingclient ###############

set(networkingclient_SRCS networkingclient.cpp)

qt5_wrap_ui(networkingclient_SRCS networkingclientview.ui)

qt5_add_dbus_interface(networkingclient_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/../src/solid/org.kde.Solid.Networking.Client.xml clientinterface)

add_executable(networkingclient ${networkingclient_SRCS})
set_target_properties(networkingclient PROPERTIES COMPILE_FLAGS -DSOLID_STATIC_DEFINE=1)

target_link_libraries(networkingclient Solid_static Qt5::Widgets)
ecm_mark_as_test(networkingclient)
########### install files ###############


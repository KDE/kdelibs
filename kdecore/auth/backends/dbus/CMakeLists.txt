cmake_minimum_required(VERSION 2.6)

project( dbus_helper_proxy )

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

# HelperProxy static library
set( helper_proxy_sources
     DBusHelperProxy.cpp )
set( helper_proxy_headers
     DBusHelperProxy.h )

qt4_add_dbus_adaptor(adaptor_srcs org.kde.auth.xml DBusHelperProxy.h Auth::DBusHelperProxy)

qt4_wrap_cpp(helper_proxy_mocs ${helper_proxy_headers} )

add_library(helper_proxy STATIC ${helper_proxy_sources} ${helper_proxy_mocs} ${adaptor_srcs})

set_target_properties(helper_proxy PROPERTIES COMPILE_FLAGS "-DQT_STATICPLUGIN -fPIC -Wall -ggdb")

install(FILES org.kde.auth.conf DESTINATION ${CMAKE_INSTALL_PREFIX}/etc/dbus-1/system.d)
install(FILES dbus_policy.stub dbus_service.stub DESTINATION ${CMAKE_INSTALL_PREFIX}/share/kauth)

cmake_minimum_required(VERSION 2.6)

project( dbus_helper_proxy )

# HelperProxy static library
set( helper_proxy_sources
     DBusHelperProxy.cpp
     DBusHelperProxyAdaptor.cpp )
set( helper_proxy_headers
     DBusHelperProxy.h
     DBusHelperProxyAdaptor.h )

qt4_wrap_cpp(helper_proxy_mocs ${helper_proxy_headers})

add_library(helper_proxy STATIC ${helper_proxy_sources} ${helper_proxy_mocs})

set_target_properties(helper_proxy PROPERTIES COMPILE_FLAGS "-DQT_STATICPLUGIN -fPIC -ggdb")
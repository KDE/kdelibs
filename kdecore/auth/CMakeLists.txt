cmake_minimum_required(VERSION 2.6)

project( libkauth )
find_package( Qt4 REQUIRED )
set(QT_USE_QTDBUS true)
include(${QT_USE_FILE})
include(DBusMacros.cmake)
include(FindPolkitQt.cmake)

include_directories(lib interfaces ${CMAKE_BINARY_DIR} ${QT_INCLUDES} ${POLKITQT_INCLUDE_DIR})

# AuthBackend static library
set( kauth_backend_sources 
     backends/policykit/Backend.cpp )
set( kauth_backend_headers 
     backends/policykit/Backend.h )
qt4_wrap_cpp(kauth_backend_mocs ${kauth_backend_headers})

add_library(kauth_backend STATIC ${kauth_backend_sources} ${kauth_backend_mocs})

set_target_properties(kauth_backend PROPERTIES COMPILE_FLAGS "-DQT_STATICPLUGIN -fPIC")

set(kauth_backend_ldflags ${POLKITQT_LIBRARIES})

# HelperProxy static library
set( helper_proxy_sources
    backends/dbus/DBusHelperProxy.cpp
    backends/dbus/DBusHelperProxyAdaptor.cpp )
set( helper_proxy_headers
    backends/dbus/DBusHelperProxy.h
    backends/dbus/DBusHelperProxyAdaptor.h )

qt4_wrap_cpp(helper_proxy_mocs ${helper_proxy_headers})

add_library(helper_proxy STATIC ${helper_proxy_sources} ${helper_proxy_mocs})

set_target_properties(helper_proxy PROPERTIES COMPILE_FLAGS "-DQT_STATICPLUGIN -fPIC")

# libkauth shared library

set( kauth_sources 
     lib/Action.cpp
     lib/ActionReply.cpp 
     lib/BackendsManager.cpp )

# TODO: Add here kauth_headers and kauth_mocs

add_library(kauth SHARED ${kauth_sources} ${kauth_mocs})
target_link_libraries(kauth kauth_backend helper_proxy ${QT_LIBRARIES} ${kauth_backend_ldflags} ${helper_proxt_ldflags})

install(TARGETS kauth DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

# Auth example application

set(kauth_example_sources
    main.cpp
    mainwindow.cpp )

set(kauth_example_headers
    mainwindow.h )

qt4_wrap_cpp(kauth_example_mocs ${kauth_example_headers})
qt4_wrap_ui(kauth_example_uis mainwindow.ui)

add_executable(kauth_example ${kauth_example_sources} ${kauth_example_mocs} ${kauth_example_uis})
target_link_libraries(kauth_example kauth ${QT_LIBRARIES} ${POLKIT_QT_LIBRARIES})

install(TARGETS kauth_example DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

# Auth example helper

qt4_wrap_cpp(kauth_helper_mocs helper/helper.h)
add_executable(kauth_helper helper/helper.cpp ${kauth_helper_mocs})
target_link_libraries(kauth_helper kauth ${QT_LIBRARIES} ${POLKIT_QT_LIBRARIES})

install(TARGETS kauth_helper DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

# Auxiliary files

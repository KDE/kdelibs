cmake_minimum_required(VERSION 2.6)

project( polkit_backend )

include_directories(${POLKITQT_INCLUDE_DIR})

# AuthBackend static library
set( kauth_backend_sources 
     PolicyKitBackend.cpp )
set( kauth_backend_headers 
     PolicyKitBackend.h )
qt4_wrap_cpp(kauth_backend_mocs ${kauth_backend_headers})

add_library(kauth_backend STATIC ${kauth_backend_sources} ${kauth_backend_mocs})

set_target_properties(kauth_backend PROPERTIES COMPILE_FLAGS "-DQT_STATICPLUGIN -fPIC -Wall -ggdb")
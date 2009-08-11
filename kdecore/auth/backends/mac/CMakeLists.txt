cmake_minimum_required(VERSION 2.6)

project( mac_backend )

# AuthBackend static library
set( kauth_backend_sources 
     AuthServicesBackend.cpp )
set( kauth_backend_headers 
     AuthServicesBackend.h )
qt4_wrap_cpp(kauth_backend_mocs ${kauth_backend_headers})

add_library(kauth_backend STATIC ${kauth_backend_sources} ${kauth_backend_mocs})

set_target_properties(kauth_backend PROPERTIES COMPILE_FLAGS "-DQT_STATICPLUGIN -fPIC -Wall -ggdb")

add_library(kauth-policy-gen-backend STATIC kauth-policy-gen-mac.cpp)

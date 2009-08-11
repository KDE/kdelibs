cmake_minimum_required(VERSION 2.6)

project( libkauth )
find_package( Qt4 REQUIRED )
set(QT_USE_QTDBUS true)
set(QT_USE_QTGUI false)
include(${QT_USE_FILE})

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules" ${CMAKE_MODULE_PATH})

if(APPLE)
    find_library(CORE_FOUNDATION_LIBRARY CoreFoundation)
    find_library(SECURITY_LIBRARY Security)
    set(KAUTH_DEPENDENCIES ${CORE_FOUNDATION_LIBRARY} ${SECURITY_LIBRARY} ${QT_QTDBUS_LIBRARY} ${QT_QTCORE_LIBRARIES})
elseif(UNIX)
    find_package(PolkitQt REQUIRED)
    set(KAUTH_DEPENDENCIES ${POLKITQT_CORE_LIBRARY} ${QT_QTDBUS_LIBRARY} ${QT_QTCORE_LIBRARY})
endif()

set(LIB_SUFFIX "" CACHE STRING "Define suffix of directory name (32/64)" )

include_directories(lib interfaces policy-gen ${CMAKE_BINARY_DIR} ${QT_INCLUDES} )

add_subdirectory(backends)

# libkauth library

set( kauth_sources 
     lib/Action.cpp
     lib/ActionReply.cpp 
     lib/BackendsManager.cpp 
     lib/HelperSupport.cpp 
     lib/ActionWatcher.cpp 
     lib/AuthBackend.cpp
     lib/HelperProxy.cpp)

set( kauth_headers
     lib/Action.h
     lib/ActionReply.h
     lib/ActionWatcher.h
     lib/BackendsManager.h
     lib/HelperSupport.h
     lib/kauth.h)

qt4_wrap_cpp(kauth_mocs lib/HelperProxy.h lib/ActionWatcher.h)

add_library(kauth SHARED ${kauth_sources} ${kauth_mocs})
target_link_libraries(kauth kauth_backend helper_proxy ${KAUTH_DEPENDENCIES}) 

add_executable(kauth-policy-gen policy-gen/policy-gen.cpp)
if(APPLE)
	target_link_libraries(kauth-policy-gen kauth-policy-gen-backend ${CORE_FOUNDATION_LIBRARY} ${SECURITY_LIBRARY} ${QT_QTCORE_LIBRARY})
elseif(UNIX)
	target_link_libraries(kauth-policy-gen kauth-policy-gen-backend ${QT_QTCORE_LIBRARY})
endif()

install(TARGETS kauth DESTINATION ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX})
install(FILES ${kauth_headers} DESTINATION
${CMAKE_INSTALL_PREFIX}/include/kauth COMPONENT Devel
)
install(TARGETS kauth-policy-gen DESTINATION ${CMAKE_INSTALL_PREFIX}/libexec)

set_target_properties(kauth PROPERTIES COMPILE_FLAGS "-Wall -ggdb")

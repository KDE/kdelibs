cmake_minimum_required(VERSION 2.6)

project( libkauth )
find_package( Qt4 REQUIRED )
set(QT_USE_QTDBUS true)
include(${QT_USE_FILE})

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

if(APPLE)
	find_library(CORE_FOUNDATION_LIBRARY CoreFoundation)
	find_library(SECURITY_LIBRARY Security)
	set(LIBRARIES ${CORE_FOUNDATION_LIBRARY} ${SECURITY_LIBRARY} ${QT_QTCORE_LIBRARIES})
elseif(UNIX)
	find_package(PolkitQt)
	set(LIBRARIES ${POLKITQT_CORE_LIBRARY} ${QT_QTCORE_LIBRARIES})
endif()

include(DBusMacros)

include_directories(lib interfaces ${CMAKE_BINARY_DIR} ${QT_INCLUDES} )

add_subdirectory(backends)

# libkauth library

set( kauth_sources 
     lib/Action.cpp
     lib/ActionReply.cpp 
     lib/BackendsManager.cpp 
     lib/HelperSupport.cpp 
     lib/ActionWatcher.cpp )

set( kauth_headers
     lib/Action.h
     lib/ActionReply.h
     lib/BackendsManager.h
     lib/HelperSupport.h
     lib/kauth.h)

# Define here ${kauth_mocs} if needed in the future
qt4_wrap_cpp(kauth_mocs lib/HelperProxy.h lib/ActionWatcher.h)

add_library(kauth SHARED ${kauth_sources} ${kauth_mocs})
target_link_libraries(kauth kauth_backend helper_proxy ${LIBRARIES}) # The last two variables should be set by the backends code, if needed

install(TARGETS kauth DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(FILES ${kauth_headers} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/kauth)

set_target_properties(kauth PROPERTIES COMPILE_FLAGS "-ggdb")

add_subdirectory(example)

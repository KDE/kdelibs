cmake_minimum_required(VERSION 2.6)

project( libkauth )
find_package( Qt4 REQUIRED )
set(QT_USE_QTDBUS true)
include(${QT_USE_FILE})

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

find_package(PolkitQt)
include(DBusMacros)

include_directories(lib interfaces ${CMAKE_BINARY_DIR} ${QT_INCLUDES} )

add_subdirectory(backends)

# libkauth library

set( kauth_sources 
     lib/Action.cpp
     lib/ActionReply.cpp 
     lib/BackendsManager.cpp 
     lib/helper_debug.cpp )

# Define here ${kauth_mocs} if needed in the future

add_library(kauth SHARED ${kauth_sources} ${kauth_mocs})
target_link_libraries(kauth kauth_backend helper_proxy ${QT_QTCORE_LIBRARIES} ${POLKITQT_CORE_LIBRARY}) # The last two variables should be set by the backends code, if needed

install(TARGETS kauth DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

set_target_properties(kauth PROPERTIES COMPILE_FLAGS "-ggdb")

add_subdirectory(example)

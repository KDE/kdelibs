cmake_minimum_required(VERSION 2.8.10.20130309)

project(kde4support)

find_package(ECM 0.0.8 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

find_package(KF5 5.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

include(ECMVersion)

# -- Maintained by scripty
ecm_version(5 0 0)
# --

include(ECMQtFramework)
include(ECMWriteVersionHeader)

find_package(Qt5 5.2.0 REQUIRED NO_MODULE COMPONENTS X11Extras Network)

remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)
remove_definitions(-DQT_NO_CAST_TO_ASCII)
remove_definitions(-DQT_NO_KEYWORDS)

ecm_write_version_header(${CMAKE_CURRENT_BINARY_DIR}/kde4support_version.h)

kde4_add_executable(kf5-config NOGUI kde-config.cpp )

target_link_libraries(kf5-config ${KDE4_KDECORE_LIBS} ${KCoreAddons_LIBRARIES} kde4support)
target_include_directories(kf5-config PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/../..")

install(TARGETS kf5-config ${INSTALL_TARGETS_DEFAULT_ARGS} )
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kde4support_version.h
        DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel )

add_subdirectory(src)
add_subdirectory(autotests)
add_subdirectory(tests)

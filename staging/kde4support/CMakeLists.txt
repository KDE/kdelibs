cmake_minimum_required(VERSION 2.8.10.1)

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
include_directories( ${KDE4_KDECORE_INCLUDES} )

find_package(Qt5Transitional REQUIRED MODULE COMPONENTS Core X11Extras)

remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)
remove_definitions(-DQT_NO_CAST_TO_ASCII)
remove_definitions(-DQT_NO_KEYWORDS)

ecm_write_version_header(${CMAKE_CURRENT_BINARY_DIR}/kde4support_version.h)

include_directories(
       ${ki18n_SOURCE_DIR}/src
       ${ki18n_BINARY_DIR}/src
       ${kde4support_SOURCE_DIR}/src/kde4support
       ${kde4support_BINARY_DIR}/src
)
kde4_add_executable(kf5-config NOGUI kde-config.cpp )

target_link_libraries(kf5-config ${KDE4_KDECORE_LIBS} ${KCoreAddons_LIBRARIES} kde4support)

install(TARGETS kf5-config ${INSTALL_TARGETS_DEFAULT_ARGS} )
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kde4support_version.h
        DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel )

add_subdirectory(src)
add_subdirectory(autotests)
add_subdirectory(tests)

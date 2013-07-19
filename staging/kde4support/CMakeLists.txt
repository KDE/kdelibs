cmake_minimum_required(VERSION 2.8.11)

project(KDE4Support)

find_package(ECM 0.0.8 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

find_package(KF5 5.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

find_package(Qt5 5.2.0 REQUIRED NO_MODULE COMPONENTS Network)

if(NOT WIN32)
    find_package(Qt5 5.2.0 REQUIRED NO_MODULE COMPONENTS X11Extras)
endif(NOT WIN32)


include(FeatureSummary)

include(ECMSetupVersion)

ecm_setup_version(5 0 0 VARIABLE_PREFIX KDE4SUPPORT
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kde4support_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KDE4SupportConfigVersion.cmake")


# QUrl
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0)

remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)
remove_definitions(-DQT_NO_CAST_TO_ASCII)
remove_definitions(-DQT_NO_KEYWORDS)

kde4_add_executable(kf5-config NOGUI kde-config.cpp )

target_link_libraries(kf5-config ${KDE4_KDECORE_LIBS} ${KCoreAddons_LIBRARIES} KDE4Support)
target_include_directories(kf5-config PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/../..")

install(TARGETS kf5-config EXPORT KDE4SupportTargets ${INSTALL_TARGETS_DEFAULT_ARGS} ) # put this for now into the export set KDE4SupportTargets, so that it exists and is not empty


# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KDE4Support")

include(CMakePackageConfigHelpers)

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KDE4SupportConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/KDE4SupportConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              PATH_VARS  INCLUDE_INSTALL_DIR CMAKE_INSTALL_PREFIX )

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/KDE4SupportConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/KDE4SupportConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"  COMPONENT Devel )

install(EXPORT KDE4SupportTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KDE4SupportTargets.cmake NAMESPACE KF5:: )

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kde4support_version.h
        DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel )

add_subdirectory(src)
add_subdirectory(autotests)
add_subdirectory(tests)


if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()

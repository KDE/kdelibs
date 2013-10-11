cmake_minimum_required(VERSION 2.8.12)

project(KDE4Support)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

find_package(KF5 5.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

find_package(Qt5 5.2.0 REQUIRED NO_MODULE COMPONENTS Network Widgets DBus Test Svg Concurrent)

if(NOT WIN32)
    find_package(Qt5 5.2.0 REQUIRED NO_MODULE COMPONENTS X11Extras)
endif(NOT WIN32)

find_package(Strigi 0.6.3 QUIET)
set_package_properties(Strigi PROPERTIES DESCRIPTION "Desktop indexing and search support"
                       URL "http://strigi.sourceforge.net"
                       TYPE OPTIONAL
                       PURPOSE ""
                      )

find_package(X11)
set(HAVE_X11 ${X11_FOUND})
if(X11_FOUND)
    set_package_properties(X11_Xrender PROPERTIES DESCRIPTION "X Rendering Extension (libXrender)"
                           URL "http://www.x.org" TYPE RECOMMENDED
                           PURPOSE "Support for compositing, rendering operations, and alpha-blending")
    set(HAVE_XRENDER ${X11_Xrender_FOUND})
endif()
if (APPLE)
   find_package(Carbon REQUIRED)
endif (APPLE)

include(FeatureSummary)
include(ECMSetupVersion)
include(ECMMarkNonGuiExecutable)

ecm_setup_version(5.0.0 VARIABLE_PREFIX KDE4SUPPORT
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kde4support_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KDE4SupportConfigVersion.cmake")

# QUrl
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0)

remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)
remove_definitions(-DQT_NO_CAST_TO_ASCII)
remove_definitions(-DQT_NO_KEYWORDS)

# This one won't be needed when CMake 2.8.13 is depended on.
add_definitions(
  -DKDE4SUPPORT_DEPRECATED_EXPORT=KDE4SUPPORT_EXPORT
)

add_executable(kf5-config kde-config.cpp )
ecm_mark_nongui_executable(kf5-config)

target_link_libraries(kf5-config KF5::KCoreAddons KI18n KConfigCore KDE4Support)
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

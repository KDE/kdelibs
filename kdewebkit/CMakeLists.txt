cmake_minimum_required(VERSION 2.8.11)

project(KDEWebKit)

find_package(ECM 0.0.8 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

set(REQUIRED_QT_VERSION "5.2")
find_package(Qt5 ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE COMPONENTS Core Xml Network Test)
find_package(KF5 5.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

include(FeatureSummary)

include(ECMSetupVersion)

ecm_setup_version(5 0 0 VARIABLE_PREFIX KUNITCONVERSION
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/KDEWebKit_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KDEWebKitConfigVersion.cmake")

remove_definitions(-DQT_NO_CAST_TO_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)
remove_definitions(-DQT_NO_KEYWORDS)

set(KDEWebKit_LIB_SRCS
    kwebwallet.cpp
    kgraphicswebview.cpp
    kwebpage.cpp
    kwebview.cpp
    kwebpluginfactory.cpp
)

add_library(KDEWebKit ${KDEWebKit_LIB_SRCS})
generate_export_header(KDEWebKit)

target_link_libraries(KDEWebKit LINK_PUBLIC Qt5::Core Qt5::Gui Qt5::WebKitWidgets
                                LINK_PRIVATE KCoreAddons KWallet KIOWidgets KParts)

set_target_properties(KDEWebKit PROPERTIES VERSION ${KDE_NON_GENERIC_LIB_VERSION} SOVERSION ${KDE_NON_GENERIC_LIB_SOVERSION})

install(TARGETS KDEWebKit EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/kdewebkit_export.h
    kwebwallet.h
    kgraphicswebview.h
    kwebpluginfactory.h
    kwebpage.h
    kwebview.h
    DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

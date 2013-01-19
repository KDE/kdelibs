cmake_minimum_required(VERSION 2.8.10.1)

project(kdeqt5staging)

find_package(ECM 0.0.5 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(ECMVersion)

# -- Maintained by scripty
ecm_version(5 0 0)
# --

include("${ECM_KDE_MODULE_DIR}/KDEInstallDirs.cmake")
include("${ECM_KDE_MODULE_DIR}/KDECMakeSettings.cmake")
include("${ECM_KDE_MODULE_DIR}/KDECompilerSettings.cmake")

include(ECMQtFramework)

find_package(Qt5Transitional REQUIRED Core)


include(FeatureSummary)

add_subdirectory(src)
add_subdirectory(autotests)


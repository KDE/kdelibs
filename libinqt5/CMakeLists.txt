cmake_minimum_required(VERSION 2.8.8)

project(inqt5)

find_package(ECM 0.0.4 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(ECMVersion)

# -- Maintained by scripty
ecm_version(5 0 0)
# --

# searching for KF5 in inqt5 would be strange, so simply include the files which contain the cmake settings etc.:
include("${ECM_KDE_MODULE_DIR}/KDEInstallDirs.cmake")
include("${ECM_KDE_MODULE_DIR}/KDECMakeSettings.cmake")
include("${ECM_KDE_MODULE_DIR}/KDECompilerSettings.cmake")

include(ECMQtFramework)

find_package(Qt4 REQUIRED QtCore)

include(${QT_USE_FILE})

include(FeatureSummary)

add_subdirectory(src)
add_subdirectory(autotests)


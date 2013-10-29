project(kded)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

set(REQUIRED_QT_VERSION "5.2")
find_package(Qt5 "${REQUIRED_QT_VERSION}" REQUIRED NO_MODULE COMPONENTS Xml Widgets DBus Network Test)
find_package(KF5 5.0.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

include(FeatureSummary)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)

if(NOT WIN32)
    find_package(Qt5 5.2.0 REQUIRED NO_MODULE COMPONENTS DBus)
endif(NOT WIN32)

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

########### next target ###############

add_subdirectory(src)



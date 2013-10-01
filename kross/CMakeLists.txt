find_package(ECM 0.0.8 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(FeatureSummary)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)

set(REQUIRED_QT_VERSION "5.2")
find_package(KF5 5.0.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)
find_package(Qt5 "${REQUIRED_QT_VERSION}" REQUIRED NO_MODULE COMPONENTS Network Widgets Xml Test)

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

find_package(Qt5Script)
find_package(Qt5UiTools)

# the core library
add_subdirectory( src )

# unittests and such stuff
add_subdirectory( tests )

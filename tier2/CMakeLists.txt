project(tier2)

find_package(ECM 0.0.6 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

find_package(KF5 5.0.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

add_subdirectory(kauth)
add_subdirectory(kconfig)

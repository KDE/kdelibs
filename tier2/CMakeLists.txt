project(tier2)

find_package(KF5 5.0.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

add_subdirectory(kauth)
add_subdirectory(kcrash)
add_subdirectory(kunitconversion)

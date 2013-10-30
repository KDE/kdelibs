
cmake_minimum_required(VERSION 2.8.12)

find_package(ECM 0.0.9 REQUIRED CONFIG)

set(CMAKE_MODULE_PATH ${ECM_KDE_MODULE_DIR})

include(KDECMakeSettings)
include(KDEInstallDirs)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/KF5ConfigVersion.cmake"
  VERSION 5.0.0
  COMPATIBILITY AnyNewerVersion
)

install(FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5Config.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5ConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_PREFIX}/KF5"
  COMPONENT Devel
)

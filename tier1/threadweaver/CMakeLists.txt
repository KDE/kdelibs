cmake_minimum_required(VERSION 2.8.10.1)

project(threadweaver)

find_package(ECM 0.0.5 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

# -- Maintained by scripty
set(THREADWEAVER_VERSION_MAJOR 5)
set(THREADWEAVER_VERSION_MINOR 0)
set(THREADWEAVER_VERSION_PATCH 0)
set(THREADWEAVER_SOVERSION ${THREADWEAVER_VERSION_MAJOR} )
set(THREADWEAVER_VERSION_STRING "${THREADWEAVER_VERSION_MAJOR}.${THREADWEAVER_VERSION_MINOR}.${THREADWEAVER_VERSION_PATCH}")
# --

find_package(KF5 5.0.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

find_package(Qt5Transitional REQUIRED Core Test)

include(FeatureSummary)

configure_file(threadweaver_version.h.in ${CMAKE_CURRENT_BINARY_DIR}/threadweaver_version.h)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/threadweaver_version.h
        DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel )

add_subdirectory(src)
add_subdirectory(autotests)
#add_subdirectory(examples)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/threadweaver")

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${CMAKE_CURRENT_LIST_DIR}/threadweaverConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/threadweaverConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
  PATH_VARS  INCLUDE_INSTALL_DIR LIB_INSTALL_DIR CMAKE_INSTALL_PREFIX
)

write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/threadweaverConfigVersion.cmake"
  VERSION ${THREADWEAVER_VERSION_STRING}
  COMPATIBILITY AnyNewerVersion
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/threadweaverConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/threadweaverConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT threadweaverTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE threadweaverTargets.cmake NAMESPACE KDE4__ )

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()

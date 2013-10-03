cmake_minimum_required(VERSION 2.8.11)

project(KHtml)

find_package(ECM 0.0.8 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
include(FeatureSummary)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)


set(REQUIRED_QT_VERSION "5.2")
find_package(Qt5 "${REQUIRED_QT_VERSION}" REQUIRED NO_MODULE COMPONENTS Widgets Network DBus PrintSupport Xml Test)
find_package(KF5 5.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

find_package(Phonon4Qt5 4.6.60 NO_MODULE)
if (NOT Phonon4Qt5_FOUND)
  return()
endif()
ecm_setup_version(5.0.0 VARIABLE_PREFIX KHTML
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/khtml_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KHtmlConfigVersion.cmake")

find_package(Perl)
set_package_properties(Perl PROPERTIES URL "http://www.perl.org" TYPE REQUIRED PURPOSE "Needed for building kdelibs")

find_package(X11)
set(HAVE_X11 ${X11_FOUND})

find_package(Phonon4Qt5 4.6.60 REQUIRED NO_MODULE)

find_package(JPEG)
set_package_properties(JPEG PROPERTIES DESCRIPTION "JPEG image format support"
                       URL "http://www.ijg.org"
                       TYPE REQUIRED
                       PURPOSE "Required by khtml"
                      )

find_package(GIF)
set_package_properties(GIF PROPERTIES DESCRIPTION "GIF image format support"
                       URL "http://sourceforge.net/projects/giflib"
                       TYPE REQUIRED
                       PURPOSE "Required by khtml"
                      )

find_package(PNG)
set_package_properties(PNG PROPERTIES DESCRIPTION "PNG image format support"
                       URL "http://www.libpng.org/pub/png"
                       TYPE REQUIRED
                       PURPOSE "Required by khtml"
                      )

if(NOT WIN32)
    find_package(Qt5 5.2.0 REQUIRED NO_MODULE COMPONENTS X11Extras)
endif(NOT WIN32)


remove_definitions(-DQT_NO_CAST_TO_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)

add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0)

add_definitions(-DENABLE_SVG)
add_definitions(-DENABLE_SVG_FONTS)
add_definitions(-DWTF_PLATFORM_QT)

set(KHTML_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(KHTML_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/src)

add_subdirectory(src)
add_subdirectory(tests)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KHtml")

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KHtmlConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KHtmlConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
  PATH_VARS  INCLUDE_INSTALL_DIR CMAKE_INSTALL_PREFIX
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KHtmlConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KHtmlConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KHtmlTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KHtmlTargets.cmake NAMESPACE KF5:: )

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/khtml_version.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()

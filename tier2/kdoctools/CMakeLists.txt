cmake_minimum_required(VERSION 2.8.12)

project(KDocTools)

# ECM setup
find_package(ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(FeatureSummary)
include(ECMSetupVersion)

set(KF5_VERSION "5.0.0")
ecm_setup_version(${KF5_VERSION} VARIABLE_PREFIX KDOCTOOLS
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kdoctools_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5DocToolsConfigVersion.cmake")

# Dependencies
set(REQUIRED_QT_VERSION 5.2.0)
find_package(Qt5Core ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)
include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings)
include(KDECMakeSettings)

if("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  # KF5 frameworks
  find_package(KF5Archive REQUIRED)
endif()

find_package(LibXslt)
set_package_properties(LibXslt PROPERTIES
                       URL "http://xmlsoft.org/XSLT"
                       TYPE REQUIRED
                       PURPOSE "Required by the KDE help system to process DocBook XML"
                      )

find_package(LibXml2)
set_package_properties(LibXml2 PROPERTIES
                       URL "http://xmlsoft.org"
                       TYPE REQUIRED
                       PURPOSE "Required by the KDE help system to process DocBook XML"
                      )


find_package(DocBookXML)

set_package_properties(DocBookXML PROPERTIES DESCRIPTION "DocBook XML"
                       URL "http://www.oasis-open.org/docbook/xml/${DOCBOOKXML_CURRENTDTD_VERSION}"
                       TYPE REQUIRED
                       PURPOSE "Required by the KDE help system to process DocBook XML"
                      )

find_package(DocBookXSL)
set_package_properties(DocBookXSL PROPERTIES DESCRIPTION "DocBook XSL"
                       URL "http://docbook.sourceforge.net/release/xsl/current/"
                       TYPE REQUIRED
                       PURPOSE "Required by the KDE help system to process DocBook XML"
                      )

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

set(_kdeBootStrapping TRUE)
set(KDOCTOOLS_CUSTOMIZATION_DIR "${CMAKE_CURRENT_BINARY_DIR}/src/customization/")
if (WIN32)
    set(KDOCTOOLS_MEINPROC_EXECUTABLE          ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/meinproc5 )
else ()
    set(KDOCTOOLS_MEINPROC_EXECUTABLE          ${CMAKE_CURRENT_BINARY_DIR}/src/meinproc5${CMAKE_EXECUTABLE_SUFFIX} )
endif ()
include(KF5DocToolsMacros.cmake)

set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KF5DocTools")
add_subdirectory(docs)
add_subdirectory(src)

install(EXPORT KF5DocToolsTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5DocToolsTargets.cmake NAMESPACE KF5:: )
configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5DocToolsConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5DocToolsConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF5DocToolsConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5DocToolsConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)


configure_file(checkXML.in.cmake ${CMAKE_CURRENT_BINARY_DIR}/checkXML )
include(ConfigureChecks.cmake)
configure_file(config-kdoctools.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kdoctools.h )

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/checkXML DESTINATION ${BIN_INSTALL_DIR} )

install( FILES
    KF5DocToolsMacros.cmake
    DESTINATION ${CMAKECONFIG_INSTALL_DIR} COMPONENT Devel )

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()

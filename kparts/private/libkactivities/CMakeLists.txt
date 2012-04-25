project(kactivities)

cmake_minimum_required(VERSION 2.8)

FIND_PACKAGE(KDE4 REQUIRED)
INCLUDE(KDE4Defaults)
INCLUDE(MacroLibrary)
INCLUDE(MacroOptionalAddSubdirectory)
INCLUDE(FindPackageHandleStandardArgs)

# =======================================================
# Information to update before to release this library.

# Library version history:
# API      ABI
# 0.1.0 => 0.1.0
# 0.1.1 => 0.1.1
# 0.2.0 => 0.2.0

# Library API version
SET(KACTIVITIES_LIB_MAJOR_VERSION "6")
SET(KACTIVITIES_LIB_MINOR_VERSION "0")
SET(KACTIVITIES_LIB_PATCH_VERSION "0")

# Suffix to add at end of version string. Usual values are:
# "-git"   : alpha code unstable from git. Do not use in production
# "-beta1" : beta1 release.
# "-beta2" : beta2 release.
# "-beta3" : beta3 release.
# "-rc"    : release candidate.
# ""       : final relase. Can be used in production.
SET(KACTIVITIES_LIB_SUFFIX_VERSION "")

# Library ABI version used by linker.
# For details : http://www.gnu.org/software/libtool/manual/libtool.html#Updating-version-info
SET(KACTIVITIES_LIB_SO_CUR_VERSION "6")
SET(KACTIVITIES_LIB_SO_REV_VERSION "0")
SET(KACTIVITIES_LIB_SO_AGE_VERSION "0")

# =======================================================
# Set env. variables accordinly.

set(KACTIVITIES_INCLUDE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/.."
    "${CMAKE_CURRENT_SOURCE_DIR}/"
    CACHE STRING
    "Location of libkactivities headers" FORCE)
set(KACTIVITIES_LIBS
    "kactivities"
    CACHE STRING
    "Location of libkactivities binary" FORCE)

SET(KACTIVITIES_LIB_VERSION_STRING "${KACTIVITIES_LIB_MAJOR_VERSION}.${KACTIVITIES_LIB_MINOR_VERSION}.${KACTIVITIES_LIB_PATCH_VERSION}${KACTIVITIES_LIB_SUFFIX_VERSION}")
SET(KACTIVITIES_LIB_VERSION_ID "0x0${KACTIVITIES_LIB_MAJOR_VERSION}0${KACTIVITIES_LIB_MINOR_VERSION}0${KACTIVITIES_LIB_PATCH_VERSION}")
SET(KACTIVITIES_LIB_SO_VERSION_STRING "${KACTIVITIES_LIB_SO_CUR_VERSION}.${KACTIVITIES_LIB_SO_REV_VERSION}.${KACTIVITIES_LIB_SO_AGE_VERSION}")

ADD_DEFINITIONS (${QT_DEFINITIONS} ${QT_QTDBUS_DEFINITIONS} ${KDE4_DEFINITIONS})
INCLUDE_DIRECTORIES (${QDBUS_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDES}
   ${KDE4_KIO_INCLUDES}
   )

set(
   kactivities_LIB_SRCS

   consumer.cpp
   controller.cpp
   info.cpp
   manager_p.cpp
   resourceinstance.cpp
   )

qt4_add_dbus_interface(
   kactivities_LIB_SRCS

   org.kde.ActivityManager.xml
   activitymanager_interface
)


kde4_add_library(
   kactivities SHARED
   ${kactivities_LIB_SRCS}
   )


set_target_properties(
   kactivities
   PROPERTIES
   VERSION ${KACTIVITIES_LIB_SO_VERSION_STRING}
   SOVERSION ${KACTIVITIES_LIB_SO_CUR_VERSION}
   )

target_link_libraries(
   kactivities
   ${KDE4_KDECORE_LIBS}
   )

## install

set(
   kactivities_LIB_HEADERS
   consumer.h
   controller.h
   info.h
   resourceinstance.h
   )

set(
   kactivities_LIB_PRETTY_HEADERS
   includes/KActivities/Consumer
   includes/KActivities/Controller
   includes/KActivities/Info
   includes/KActivities/ResourceInstance
   )

install(
   FILES ${kactivities_LIB_HEADERS}
   DESTINATION ${INCLUDE_INSTALL_DIR}/kactivities
   COMPONENT Devel
   )

install(
   FILES ${kactivities_LIB_PRETTY_HEADERS}
   DESTINATION ${INCLUDE_INSTALL_DIR}/KDE/KActivities
   COMPONENT Devel
   )

install(
   TARGETS kactivities
   EXPORT kdelibsLibraryTargets
   ${INSTALL_TARGETS_DEFAULT_ARGS}
   )

IF(NOT WIN32)
  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/libkactivities.pc.cmake ${CMAKE_CURRENT_BINARY_DIR}/libkactivities.pc)
  INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/libkactivities.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig )
ENDIF(NOT WIN32)

CONFIGURE_FILE(
    KActivitiesConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/KActivitiesConfig.cmake @ONLY
)

INSTALL(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/KActivitiesConfig.cmake
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}/cmake/KActivities
)


cmake_minimum_required(VERSION 2.8.12)

project(KDE4Support)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake )

include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)
include(FeatureSummary)

set(KF5_VERSION "5.0.0")
set(QT_REQUIRED_VERSION 5.2.0)
find_package(Qt5 ${QT_REQUIRED_VERSION} CONFIG REQUIRED Network Widgets DBus Test Svg Concurrent PrintSupport Designer)

if(NOT WIN32)
    find_package(Qt5 ${QT_REQUIRED_VERSION} CONFIG REQUIRED X11Extras)
endif()

if("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    # add dependencies
    find_package(KF5I18n ${KF5_VERSION} REQUIRED)
    find_package(KF5GuiAddons ${KF5_VERSION} REQUIRED)
    find_package(KF5WidgetsAddons ${KF5_VERSION} REQUIRED)
    find_package(KF5DBusAddons ${KF5_VERSION} REQUIRED)
    find_package(KF5Config ${KF5_VERSION} REQUIRED)
    find_package(KF5ConfigWidgets ${KF5_VERSION} REQUIRED)
    find_package(KF5ItemViews ${KF5_VERSION} REQUIRED)
    find_package(KF5IconThemes ${KF5_VERSION} REQUIRED)
    find_package(KF5UnitConversion ${KF5_VERSION} REQUIRED)
    find_package(KF5WindowSystem ${KF5_VERSION} REQUIRED)
    find_package(KF5Service ${KF5_VERSION} REQUIRED)
    find_package(KF5Completion ${KF5_VERSION} REQUIRED)
    find_package(KF5TextWidgets ${KF5_VERSION} REQUIRED)
    find_package(KF5XmlGui ${KF5_VERSION} REQUIRED)
    find_package(KF5Bookmarks ${KF5_VERSION} REQUIRED)
    find_package(KF5KIO ${KF5_VERSION} REQUIRED)
    find_package(KF5DocTools ${KF5_VERSION} REQUIRED)
    find_package(KF5Crash ${KF5_VERSION} REQUIRED)
    find_package(KF5Notifications ${KF5_VERSION} REQUIRED)
    find_package(KF5Parts ${KF5_VERSION} REQUIRED)
    find_package(KDEWidgets ${KF5_VERSION} REQUIRED)
    find_package(KF5GlobalAccel ${KF5_VERSION} REQUIRED)
else()
    set(_kdeBootStrapping TRUE)
    include(${CMAKE_SOURCE_DIR}/tier2/kdoctools/KF5DocToolsMacros.cmake)
    set(KDOCTOOLS_CUSTOMIZATION_DIR "${KDocTools_BINARY_DIR}/src/customization/")
    set(KDEWIDGETS_MAKEKDEWIDGETS_EXECUTABLE ${CMAKE_BINARY_DIR}/tier3/kdewidgets/src/makekdewidgets${CMAKE_EXECUTABLE_SUFFIX} )
    include(${CMAKE_SOURCE_DIR}/tier3/kdewidgets/KDEWidgetsMacros.cmake)
endif()

# find_package(Strigi 0.6.3 QUIET)
set_package_properties(Strigi PROPERTIES DESCRIPTION "Desktop indexing and search support"
                       URL "http://strigi.sourceforge.net"
                       TYPE OPTIONAL
                       PURPOSE ""
                      )

find_package(X11)
set(HAVE_X11 ${X11_FOUND})
if(X11_FOUND)
    set_package_properties(X11_Xrender PROPERTIES DESCRIPTION "X Rendering Extension (libXrender)"
                           URL "http://www.x.org" TYPE RECOMMENDED
                           PURPOSE "Support for compositing, rendering operations, and alpha-blending")
    set(HAVE_XRENDER ${X11_Xrender_FOUND})
endif()
if (APPLE)
   find_package(Carbon REQUIRED)
endif ()

include(ECMSetupVersion)
include(ECMMarkNonGuiExecutable)

ecm_setup_version(${KF5_VERSION} VARIABLE_PREFIX KDE4SUPPORT
                                 VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kde4support_version.h"
                                 PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KDE4SupportConfigVersion.cmake")

# QUrl
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0)

remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)
remove_definitions(-DQT_NO_CAST_TO_ASCII)

# This one won't be needed when CMake 2.8.13 is depended on.
add_definitions(
  -DKDE4SUPPORT_DEPRECATED= # if KDE4SUPPORT_DEPRECATED is already defined the other deprecated macros won't be defined
  -DKDE4SUPPORT_DEPRECATED_NO_EXPORT=KDE4SUPPORT_NO_EXPORT
  -DKDE4SUPPORT_DEPRECATED_EXPORT=KDE4SUPPORT_EXPORT
)

add_executable(kf5-config kde-config.cpp )
ecm_mark_nongui_executable(kf5-config)

target_link_libraries(kf5-config KF5::CoreAddons KF5::I18n KF5::ConfigCore KDE4Support)
target_include_directories(kf5-config PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/../..")

install(TARGETS kf5-config EXPORT KDE4SupportTargets ${INSTALL_TARGETS_DEFAULT_ARGS} ) # put this for now into the export set KDE4SupportTargets, so that it exists and is not empty


# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KDE4Support")

include(CMakePackageConfigHelpers)

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KDE4SupportConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/KDE4SupportConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              )

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/KDE4SupportConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/KDE4SupportConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"  COMPONENT Devel )

install(EXPORT KDE4SupportTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KDE4SupportTargets.cmake NAMESPACE KF5:: )

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kde4support_version.h
        DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel )

add_subdirectory(cmake)
add_subdirectory(docs)
add_subdirectory(src)
add_subdirectory(autotests)
add_subdirectory(tests)


set (KDE_VERSION_MAJOR 4)
set (KDE_VERSION_MINOR 90)
set (KDE_VERSION_RELEASE 00)
set (KDE_VERSION "${KDE_VERSION_MAJOR}.${KDE_VERSION_MINOR}.${KDE_VERSION_RELEASE}" )
# For git master, use this:
set (KDE_VERSION_STRING "${KDE_VERSION} (KDE Frameworks >= 20121130)")
# For a stable release, use this:
#set (KDE_VERSION_STRING "${KDE_VERSION}")

include(CreateKDELibsDependenciesFile.cmake)
set(CMAKECONFIG_INSTALL_DIR ${LIB_INSTALL_DIR}/cmake/KDELibs4)

# Used in configure_file() and install(EXPORT)
set(KDE4_TARGET_PREFIX KDE4__)

################# install files #################

install( FILES ${CMAKE_CURRENT_BINARY_DIR}/KDELibsDependencies.cmake
               ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/kde4_cmake_uninstall.cmake.in
               ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/Win32.Manifest.in
         DESTINATION ${CMAKECONFIG_INSTALL_DIR} COMPONENT Devel)

# run a script before installing the exports files which deletes previously installed
# configuration specific export files KDELibs4(Library|Tools)Targets-<config>.cmake
# if the main exports file KDELibs4(Library|Tools)Targets.cmake has changed. This makes sure
# that this main file doesn't include older and different configuration specific exports files,
# which might have a different set of targets or targets with different names.
# The code for installing the exports files will soon go into a macro. Alex
install(CODE "set(EXPORT_FILES KDELibs4LibraryTargets.cmake)"
        CODE "set(EXPORT_INSTALL_DIR \"${CMAKECONFIG_INSTALL_DIR}\")"
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/check_installed_exports_file.cmake" )
install( EXPORT KDE4SupportTargets DESTINATION ${CMAKECONFIG_INSTALL_DIR} NAMESPACE ${KDE4_TARGET_PREFIX} FILE KDELibs4LibraryTargets.cmake  COMPONENT Devel)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()

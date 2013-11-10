project(KInit)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

set(REQUIRED_QT_VERSION "5.2")
find_package(Qt5 "${REQUIRED_QT_VERSION}" CONFIG REQUIRED Core Gui Widgets DBus)
include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)

include(FeatureSummary)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)


ecm_setup_version(5.0.0 VARIABLE_PREFIX KINIT
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kinit_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KInitConfigVersion.cmake")


########### Configuring (mainly for proctitle) ###########
include(ConfigureChecks.cmake)

#optional features
find_package(X11)
set(HAVE_X11 ${X11_FOUND})

# used by 4 executables in this module
if(HAVE_SOCKET_LIBRARY)
  set(KINIT_SOCKET_LIBRARY socket)
else(HAVE_SOCKET_LIBRARY)
  set(KINIT_SOCKET_LIBRARY)
endif(HAVE_SOCKET_LIBRARY)

set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KInit")

add_subdirectory(src)

install( FILES
    KInitMacros.cmake KInitConfig.cmake
    DESTINATION ${CMAKECONFIG_INSTALL_DIR} COMPONENT Devel )

install(FILES
        kde5init_dummy.cpp.in
        kde5init_win32lib_dummy.cpp.in

        DESTINATION ${CMAKECONFIG_INSTALL_DIR} COMPONENT Devel)

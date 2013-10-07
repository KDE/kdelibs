project(KInit)

find_package(ECM 0.0.8 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

set(REQUIRED_QT_VERSION "5.2")
find_package(Qt5 "${REQUIRED_QT_VERSION}" REQUIRED NO_MODULE COMPONENTS Core Gui Widgets DBus)
find_package(KF5 5.0.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

include(FeatureSummary)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)

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

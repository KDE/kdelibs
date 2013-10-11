project(kded)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

set(REQUIRED_QT_VERSION "5.2")
find_package(Qt5 "${REQUIRED_QT_VERSION}" REQUIRED NO_MODULE COMPONENTS Xml Widgets DBus Network Test)
find_package(KF5 5.0.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

include(FeatureSummary)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)

if(NOT WIN32)
    find_package(Qt5 5.2.0 REQUIRED NO_MODULE COMPONENTS X11Extras)
endif(NOT WIN32)

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

add_subdirectory( tests )

if(KDE_PLATFORM_FEATURE_BINARY_COMPATIBLE_FEATURE_REDUCTION)
    set(KBUILDSYCOCA_NO_KCRASH TRUE)
endif(KDE_PLATFORM_FEATURE_BINARY_COMPATIBLE_FEATURE_REDUCTION)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-kded.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kded.h)

find_package(X11)
set(HAVE_X11 ${X11_FOUND})

########### next target ###############

set(kded_KDEINIT_SRCS kded.cpp kdedadaptor.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kded5.xml # just so that it gets generated
)


kf5_add_kdeinit_executable(kded5 ${kded_KDEINIT_SRCS})

if (APPLE)
    set_target_properties(kded5 PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.template)
    set_target_properties(kded5 PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER "org.kded.kded5")
    set_target_properties(kded5 PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "KDE Daemon")
endif (APPLE)

target_link_libraries(kdeinit_kded5 Qt5::X11Extras ${KDE4_KIO_LIBS} ${X11_LIBRARIES} KF5::KCoreAddons ${KDBusAddons_LIBRARIES} KI18n KCrash KWindowSystem)

install(TARGETS kdeinit_kded5 ${INSTALL_TARGETS_DEFAULT_ARGS})

target_link_libraries( kded5 kdeinit_kded5)
install(TARGETS kded5 ${INSTALL_TARGETS_DEFAULT_ARGS} )


qt5_generate_dbus_interface( kdedadaptor.h org.kde.kded5.xml )
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kded5.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})

#include(MacroDBusAddActivationService) # for dbus_add_activation_service

macro(dbus_add_activation_service _sources)
    foreach (_i ${_sources})
        get_filename_component(_service_file ${_i} ABSOLUTE)
        string(REGEX REPLACE "\\.service.*$" ".service" _output_file ${_i})
        set(_target ${CMAKE_CURRENT_BINARY_DIR}/${_output_file})
        configure_file(${_service_file} ${_target})
        install(FILES ${_target} DESTINATION ${DBUS_SERVICES_INSTALL_DIR} )
    endforeach (_i ${ARGN})
endmacro(dbus_add_activation_service _sources)
dbus_add_activation_service(org.kde.kded5.service.in)

########### next target ###############

set(kbuildsycoca_KDEINIT_SRCS
   kbuildsycoca.cpp
   kbuildmimetypefactory.cpp
   kmimeassociations.cpp
   kbuildservicetypefactory.cpp
   kbuildservicefactory.cpp
   kbuildservicegroupfactory.cpp
   kctimefactory.cpp
   vfolder_menu.cpp
   )



# We need to add a '4' so that kde3 apps running kbuildsycoca don't run the kde4 one.
kf5_add_kdeinit_executable( kbuildsycoca5 NOGUI ${kbuildsycoca_KDEINIT_SRCS})

target_link_libraries(kdeinit_kbuildsycoca5
   KF5::KCoreAddons
   KService
   KI18n
   Qt5::Xml
   Qt5::DBus
)
if(NOT KBUILDSYCOCA_NO_KCRASH)
   target_link_libraries(kdeinit_kbuildsycoca5 KCrash)
endif()

install(TARGETS kdeinit_kbuildsycoca5 ${INSTALL_TARGETS_DEFAULT_ARGS})

target_link_libraries( kbuildsycoca5 kdeinit_kbuildsycoca5 )
install(TARGETS kbuildsycoca5 ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############
if (WIN32)
install( FILES applications.menu  DESTINATION  ${SHARE_INSTALL_PREFIX}/xdg/menus )
else (WIN32)
install( FILES applications.menu  DESTINATION  ${SYSCONF_INSTALL_DIR}/xdg/menus )
endif (WIN32)
install( FILES kdedmodule.desktop DESTINATION  ${SERVICETYPES_INSTALL_DIR} )



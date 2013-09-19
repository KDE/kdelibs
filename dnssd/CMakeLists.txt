cmake_minimum_required(VERSION 2.8.11)

project(KDNSSD)

find_package(ECM 0.0.8 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

find_package(KF5 5.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

set(REQUIRED_QT_VERSION "5.2.0")
find_package(Qt5Widgets ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)

include(FeatureSummary)

include(ECMSetupVersion)

ecm_setup_version(5 0 0 VARIABLE_PREFIX KDNSSD
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kdnssd_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KDNSSDConfigVersion.cmake")

configure_file(config-dnssd.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-dnssd.h )

remove_definitions(-DQT_NO_CAST_TO_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)

find_package(Avahi)
macro_bool_to_01(AVAHI_FOUND HAVE_AVAHI)
set_package_properties(Avahi PROPERTIES DESCRIPTION "Facilities for service discovery on a local network (DNSSD)"
                       URL "http://avahi.org"
                       TYPE OPTIONAL
                       PURPOSE "Either Avahi or DNSSD is required for KDE applications to make use of multicast DNS/DNS-SD service discovery"
                      )

set(kdnssd_LIB_SRCS  servicebase.cpp servicemodel.cpp domainmodel.cpp)
kconfig_add_kcfg_files(kdnssd_LIB_SRCS settings.kcfgc)

if (HAVE_AVAHI) 
   include_directories( ${AVAHI_INCLUDE_DIR} )
   set(kdnssd_LIB_SRCS ${kdnssd_LIB_SRCS}
      avahi-domainbrowser.cpp
      avahi-servicebrowser.cpp
      avahi-remoteservice.cpp
      avahi-publicservice.cpp
      avahi-servicetypebrowser.cpp
      avahi_server_interface.cpp
      avahi_serviceresolver_interface.cpp
      avahi_entrygroup_interface.cpp
      )
   qt5_add_dbus_interface (kdnssd_LIB_SRCS org.freedesktop.Avahi.DomainBrowser.xml avahi_domainbrowser_interface )
   qt5_add_dbus_interface (kdnssd_LIB_SRCS org.freedesktop.Avahi.ServiceBrowser.xml avahi_servicebrowser_interface )
   qt5_add_dbus_interface (kdnssd_LIB_SRCS org.freedesktop.Avahi.ServiceTypeBrowser.xml avahi_servicetypebrowser_interface )
else (HAVE_AVAHI)

   find_package(DNSSD)
   macro_bool_to_01(DNSSD_FOUND HAVE_DNSSD)
   set_package_properties(DNSSD PROPERTIES DESCRIPTION "Facilities for service discovery on a local network"
                          URL "http://avahi.org"
                          TYPE OPTIONAL
                          PURPOSE "Either Avahi or DNSSD is required for KDE applications to make use of multicast DNS/DNS-SD service discovery"
                         )

   if (HAVE_DNSSD)
      include_directories( ${DNSSD_INCLUDE_DIR} )
      set(kdnssd_LIB_SRCS ${kdnssd_LIB_SRCS}
         mdnsd-domainbrowser.cpp
         mdnsd-remoteservice.cpp
         mdnsd-publicservice.cpp
         mdnsd-responder.cpp
         mdnsd-servicebrowser.cpp
         mdnsd-servicetypebrowser.cpp
         )
   else (HAVE_DNSSD)
      set(kdnssd_LIB_SRCS ${kdnssd_LIB_SRCS}
         dummy-domainbrowser.cpp
         dummy-remoteservice.cpp
         dummy-publicservice.cpp
         dummy-servicebrowser.cpp
         dummy-servicetypebrowser.cpp
         )

   endif (HAVE_DNSSD)
endif (HAVE_AVAHI)


########### next target ###############


add_library(KDNSSD SHARED ${kdnssd_LIB_SRCS})
generate_export_header(KDNSSD
  EXPORT_FILE_NAME dnssd_export.h
)

target_link_libraries(KDNSSD LINK_PUBLIC Qt5::Network Qt5::DBus
                             LINK_PRIVATE KI18n KConfigCore)
if (HAVE_DNSSD)
  target_link_libraries(KDNSSD LINK_PRIVATE ${DNSSD_LIBRARIES})
endif (HAVE_DNSSD)

target_include_directories(KDNSSD PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..;${CMAKE_CURRENT_BINARY_DIR}/..>")

set_target_properties(KDNSSD PROPERTIES
   VERSION "${KDNSSD_VERSION_STRING}"
   SOVERSION "${KDNSSD_SOVERSION}"
)
install(TARGETS KDNSSD EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############
install( FILES  ${CMAKE_CURRENT_BINARY_DIR}/dnssd_export.h
                ${CMAKE_CURRENT_BINARY_DIR}/settings.h
                domainbrowser.h
                remoteservice.h
                servicetypebrowser.h
                publicservice.h
                servicebase.h
                servicebrowser.h
                servicemodel.h
                domainmodel.h
         DESTINATION  ${INCLUDE_INSTALL_DIR}/dnssd COMPONENT Devel)


if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()


set(kdnssd_LIB_SRCS  servicebase.cpp servicemodel.cpp domainmodel.cpp)
kconfig_add_kcfg_files(kdnssd_LIB_SRCS settings.kcfgc)

set(kdnssd_HEADERS
    domainbrowser.h
    remoteservice.h
    servicetypebrowser.h
    publicservice.h
    servicebase.h
    servicebrowser.h
    servicemodel.h
    domainmodel.h
)

foreach(_KDNSSD_HEADER ${kdnssd_HEADERS})
    set(HEADER_NAME ${KDNSSD_BINARY_DIR}/dnssd/${_KDNSSD_HEADER})
    if (NOT EXISTS ${HEADER_NAME})
        file(WRITE ${HEADER_NAME} "#include \"${CMAKE_CURRENT_SOURCE_DIR}/${_KDNSSD_HEADER}\"\n")
    endif()
endforeach()

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


add_library(KDNSSD SHARED ${kdnssd_LIB_SRCS})
generate_export_header(KDNSSD
  EXPORT_FILE_NAME ${KDNSSD_BINARY_DIR}/dnssd/dnssd_export.h
)

target_link_libraries(KDNSSD LINK_PUBLIC Qt5::Network Qt5::DBus
                             LINK_PRIVATE KI18n KConfigCore)
if (HAVE_DNSSD)
  target_link_libraries(KDNSSD LINK_PRIVATE ${DNSSD_LIBRARIES})
endif (HAVE_DNSSD)

target_include_directories(KDNSSD PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR};${CMAKE_CURRENT_BINARY_DIR};${KDNSSD_BINARY_DIR}>")

set_target_properties(KDNSSD PROPERTIES
   VERSION "${KDNSSD_VERSION_STRING}"
   SOVERSION "${KDNSSD_SOVERSION}"
)
install(TARGETS KDNSSD EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############
install( FILES  ${CMAKE_CURRENT_BINARY_DIR}/dnssd_export.h
                ${CMAKE_CURRENT_BINARY_DIR}/settings.h
                ${kdnssd_HEADERS}
         DESTINATION  ${INCLUDE_INSTALL_DIR}/dnssd COMPONENT Devel)
         

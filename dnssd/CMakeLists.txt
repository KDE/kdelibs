project(dnssd)
macro_optional_find_package(Avahi)
macro_bool_to_01(AVAHI_FOUND HAVE_AVAHI)
set_package_properties(Avahi PROPERTIES DESCRIPTION "Facilities for service discovery on a local network (DNSSD)"
                       URL "http://avahi.org"
                       TYPE OPTIONAL
                       PURPOSE "Either Avahi or DNSSD is required for KDE applications to make use of multicast DNS/DNS-SD service discovery"
                      )

if(KDE_PLATFORM_FEATURE_BINARY_INCOMPATIBLE_FEATURE_REDUCTION)
   include_directories( ${KDE4_KDECORE_INCLUDES} )
   set(KDNSSD_BASE_LIBS ${KDE4_KDECORE_LIBS})
else(KDE_PLATFORM_FEATURE_BINARY_INCOMPATIBLE_FEATURE_REDUCTION)
   include_directories( ${KDE4_KDEUI_INCLUDES} )
   set(KDNSSD_BASE_LIBS ${KDE4_KDEUI_LIBS})
endif(KDE_PLATFORM_FEATURE_BINARY_INCOMPATIBLE_FEATURE_REDUCTION)

include(CreateKcfgFiles.cmake)

set(kdnssd_LIB_SRCS  servicebase.cpp servicemodel.cpp domainmodel.cpp)
kde4_add_kcfg_files(kdnssd_LIB_SRCS ${CMAKE_CURRENT_BINARY_DIR}/settings.kcfgc )

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
   qt4_add_dbus_interface (kdnssd_LIB_SRCS org.freedesktop.Avahi.DomainBrowser.xml avahi_domainbrowser_interface )
   qt4_add_dbus_interface (kdnssd_LIB_SRCS org.freedesktop.Avahi.ServiceBrowser.xml avahi_servicebrowser_interface )
   qt4_add_dbus_interface (kdnssd_LIB_SRCS org.freedesktop.Avahi.ServiceTypeBrowser.xml avahi_servicetypebrowser_interface )
else (HAVE_AVAHI)

   macro_optional_find_package(DNSSD)
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


kde4_add_library(kdnssd ${LIBRARY_TYPE} ${kdnssd_LIB_SRCS})

# KDE 5: don't link against KDEUI_LIBS (when settings.kfgc is changed)
if (HAVE_DNSSD)
  target_link_libraries(kdnssd ${KDNSSD_BASE_LIBS} ${DNSSD_LIBRARIES} ${QT_QTNETWORK_LIBRARY})
else (HAVE_DNSSD)
  target_link_libraries(kdnssd ${KDNSSD_BASE_LIBS} ${QT_QTNETWORK_LIBRARY})
endif (HAVE_DNSSD)

target_link_libraries(kdnssd  LINK_INTERFACE_LIBRARIES kdecore ${QT_QTCORE_LIBRARY} )

set_target_properties(kdnssd PROPERTIES
   VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
)
install(TARGETS kdnssd EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############
install( FILES dnssd_export.h domainbrowser.h remoteservice.h servicetypebrowser.h publicservice.h 
               servicebase.h servicebrowser.h  servicemodel.h domainmodel.h ${CMAKE_CURRENT_BINARY_DIR}/settings.h
         DESTINATION  ${INCLUDE_INSTALL_DIR}/dnssd COMPONENT Devel)






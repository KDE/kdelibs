
project(dnssd)
macro_optional_find_package(DNSSD)
macro_bool_to_01(DNSSD_FOUND HAVE_DNSSD)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-dnssd.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-dnssd.h )

include_directories( ${KDE4_KDEUI_INCLUDES} )

if (HAVE_AVAHI) 
  set(kdnssd_LIB_SRCS
   servicebase.cpp
   avahi-domainbrowser.cpp
   avahi-servicebrowser.cpp
   avahi-remoteservice.cpp
   avahi-publicservice.cpp
   )
   qt4_add_dbus_interface (kdnssd_LIB_SRCS org.freedesktop.Avahi.Server.xml avahi_server_interface )
   qt4_add_dbus_interface (kdnssd_LIB_SRCS org.freedesktop.Avahi.DomainBrowser.xml avahi_domainbrowser_interface )
   qt4_add_dbus_interface (kdnssd_LIB_SRCS org.freedesktop.Avahi.ServiceBrowser.xml avahi_servicebrowser_interface )
   qt4_add_dbus_interface (kdnssd_LIB_SRCS org.freedesktop.Avahi.ServiceResolver.xml avahi_serviceresolver_interface )
   qt4_add_dbus_interface (kdnssd_LIB_SRCS org.freedesktop.Avahi.EntryGroup.xml avahi_entrygroup_interface )
else (HAVE_AVAHI)
    
if (HAVE_DNSSD)
  include_directories( ${DNSSD_INCLUDE_DIR} )
  set(kdnssd_LIB_SRCS
   servicebase.cpp
   mdnsd-domainbrowser.cpp
   mdnsd-remoteservice.cpp
   mdnsd-publicservice.cpp
   mdnsd-responder.cpp
   mdnsd-servicebrowser.cpp
   )
else (HAVE_DNSSD)
  set(kdnssd_LIB_SRCS
   servicebase.cpp
   dummy-domainbrowser.cpp
   dummy-remoteservice.cpp
   dummy-publicservice.cpp
   dummy-servicebrowser.cpp
   )

endif (HAVE_DNSSD)
endif (HAVE_AVAHI)

########### next target ###############



kde4_automoc(${kdnssd_LIB_SRCS})

set( kdnssd_KCFG_SRCS settings.kcfgc )

kde4_add_kcfg_files(kdnssd_LIB_SRCS ${kdnssd_KCFG_SRCS})

kde4_add_library(kdnssd SHARED ${kdnssd_LIB_SRCS})

if (HAVE_DNSSD)
  target_link_libraries(kdnssd ${KDE4_KDEUI_LIBS} ${DNSSD_LIBRARIES} )
else (HAVE_DNSSD)
  target_link_libraries(kdnssd ${KDE4_KDECORE_LIBS})
endif (HAVE_DNSSD)

set_target_properties(kdnssd PROPERTIES VERSION 2.0.0 SOVERSION 2 )
install(TARGETS kdnssd  DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############
install( FILES dnssd_export.h domainbrowser.h remoteservice.h   publicservice.h servicebase.h servicebrowser.h ${CMAKE_CURRENT_BINARY_DIR}/settings.h  DESTINATION  ${INCLUDE_INSTALL_DIR}/dnssd )





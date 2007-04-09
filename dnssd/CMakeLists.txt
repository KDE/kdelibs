
project(dnssd)
macro_optional_find_package(DNSSD)
macro_bool_to_01(DNSSD_FOUND HAVE_DNSSD)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-dnssd.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-dnssd.h )

include_directories( ${KDE4_KDEUI_INCLUDES} )
set(kdnssd_LIB_SRCS    servicebase.cpp )
kde4_add_kcfg_files(kdnssd_LIB_SRCS settings.kcfgc )

if (HAVE_AVAHI) 
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



kde4_automoc(${kdnssd_LIB_SRCS})

kde4_add_library(kdnssd SHARED ${kdnssd_LIB_SRCS})

if (HAVE_DNSSD)
  target_link_libraries(kdnssd ${KDE4_KDEUI_LIBS} ${DNSSD_LIBRARIES} )
else (HAVE_DNSSD)
  target_link_libraries(kdnssd ${KDE4_KDEUI_LIBS})
endif (HAVE_DNSSD)

set_target_properties(kdnssd PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kdnssd  DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############
install( FILES dnssd_export.h domainbrowser.h remoteservice.h servicetypebrowser.h  publicservice.h servicebase.h servicebrowser.h ${CMAKE_CURRENT_BINARY_DIR}/settings.h  DESTINATION  ${INCLUDE_INSTALL_DIR}/dnssd )





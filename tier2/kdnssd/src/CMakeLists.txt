set(kdnssd_LIB_SRCS  servicebase.cpp servicemodel.cpp domainmodel.cpp)

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
    set(HEADER_NAME ${KDNSSD_BINARY_DIR}/kdnssd/${_KDNSSD_HEADER})
    if (NOT EXISTS ${HEADER_NAME})
        file(WRITE ${HEADER_NAME} "#include \"${CMAKE_CURRENT_SOURCE_DIR}/${_KDNSSD_HEADER}\"\n")
    endif()
endforeach()

if (AVAHI_FOUND)
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
elseif (DNSSD_FOUND)
    kconfig_add_kcfg_files(kdnssd_LIB_SRCS settings.kcfgc)
    include_directories( ${DNSSD_INCLUDE_DIR} )
    set(kdnssd_LIB_SRCS ${kdnssd_LIB_SRCS}
        mdnsd-domainbrowser.cpp
        mdnsd-remoteservice.cpp
        mdnsd-publicservice.cpp
        mdnsd-responder.cpp
        mdnsd-servicebrowser.cpp
        mdnsd-servicetypebrowser.cpp
    )
else ()
    set(kdnssd_LIB_SRCS ${kdnssd_LIB_SRCS}
        dummy-domainbrowser.cpp
        dummy-remoteservice.cpp
        dummy-publicservice.cpp
        dummy-servicebrowser.cpp
        dummy-servicetypebrowser.cpp
    )

endif ()

add_library(KF5DNSSD SHARED ${kdnssd_LIB_SRCS})
generate_export_header(KF5DNSSD
  BASE_NAME KDNSSD
  EXPORT_FILE_NAME ${KDNSSD_BINARY_DIR}/kdnssd/kdnssd_export.h
)

target_link_libraries(KF5DNSSD PUBLIC Qt5::Network)
target_link_libraries(KF5DNSSD PRIVATE Qt5::DBus)

if (DNSSD_FOUND)
  target_link_libraries(KF5DNSSD PRIVATE ${DNSSD_LIBRARIES}
                                            KF5::ConfigCore # settings.kcfgc
                       )
endif ()

target_include_directories(KF5DNSSD PUBLIC "$<BUILD_INTERFACE:${KDNSSD_BINARY_DIR}>")

set_target_properties(KF5DNSSD PROPERTIES
   VERSION "${KDNSSD_VERSION_STRING}"
   SOVERSION "${KDNSSD_SOVERSION}"
   EXPORT_NAME DNSSD
)
install(TARGETS KF5DNSSD EXPORT KF5DNSSDTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############
install( FILES  ${KDNSSD_BINARY_DIR}/kdnssd/kdnssd_export.h
                ${kdnssd_HEADERS}
         DESTINATION  ${INCLUDE_INSTALL_DIR}/kdnssd COMPONENT Devel)

if (DNSSD_FOUND)
    install( FILES ${CMAKE_CURRENT_BINARY_DIR}/settings.h
             DESTINATION  ${INCLUDE_INSTALL_DIR}/kdnssd COMPONENT Devel)
endif ()

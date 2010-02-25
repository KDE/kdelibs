include_directories(
)

set( kupnp_LIB_SRCS
  service.cpp
  device.cpp
  devicebrowser_p.cpp
  devicebrowser.cpp
)
set( kupnp_LIB_HDRS
  upnp_export.h
  service.h
  device.h
  devicebrowser.h
)

# http://techbase.kde.org/Development/Tutorials/D-Bus/Accessing_Interfaces also adds:
#PKGCONFIG_GETVAR(dbus-1 prefix DBUS_PREFIX)
#set(network_xml ${DBUS_PREFIX}/interfaces/org.foo.bar.xml)
qt4_add_dbus_interface( kupnp_LIB_SRCS  org.Coherence.xml coherence_interface )


kde4_add_library( kupnp  SHARED ${kupnp_LIB_SRCS} )

# If making the lib SHARED again, uncomment again:
target_link_libraries( kupnp
  ${QT_QTDBUS_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${KDE4_KDECORE_LIBS})
set_target_properties( kupnp  PROPERTIES
  VERSION ${GENERIC_LIB_VERSION}
  SOVERSION ${GENERIC_LIB_SOVERSION}
)

install( TARGETS kupnp  ${INSTALL_TARGETS_DEFAULT_ARGS} )

install( FILES ${kupnp_LIB_HDRS}  DESTINATION ${INCLUDE_INSTALL_DIR}/upnp  COMPONENT Devel )

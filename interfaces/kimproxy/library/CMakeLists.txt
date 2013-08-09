add_subdirectory( icons )

########### next target ###############

set(kimproxy_LIB_SRCS
  kimproxy.cpp
)

qt5_add_dbus_adaptor( kimproxy_LIB_SRCS ../interface/org.kde.KIMIface.xml kimproxy.h  KIMProxy)

qt5_add_dbus_interface(kimproxy_LIB_SRCS ../interface/org.kde.KIMIface.xml kimiface )


kde4_add_library(kimproxy ${LIBRARY_TYPE} ${kimproxy_LIB_SRCS})
generate_export_header(kimproxy)

target_link_libraries(kimproxy  ${KDE4_KIO_LIBS} KDE4Support)
target_link_libraries(kimproxy  LINK_INTERFACE_LIBRARIES Qt5::Core )

set_target_properties(kimproxy PROPERTIES
   VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
)

########### install files ###############

install(TARGETS kimproxy  EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})
install( FILES ${CMAKE_CURRENT_BINARY_DIR}/kimproxy_export.h kimproxy.h DESTINATION  ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

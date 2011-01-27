PROJECT( ksecretserviceclient )

FIND_PACKAGE (QCA2 REQUIRED)

include_directories (
    ${KDE4_INCLUDES}
    ${QCA2_INCLUDE_DIR} 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/daemon/frontend/secret
)

set (client_lib_SRCS
    ksecretservice.cpp
    ksecretservicecollection.cpp
    ../daemon/frontend/secret/adaptors/secret.cpp
    ../daemon/frontend/secret/adaptors/dbustypes.cpp
)

MACRO(KSS_ADD_DBUS_INTERFACE _sources _interface _basename)
  GET_FILENAME_COMPONENT(_infile ${_interface} ABSOLUTE)
  SET(_header ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.h)
  SET(_impl   ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.cpp)
  SET(_moc    ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.moc)

  ADD_CUSTOM_COMMAND(OUTPUT ${_impl} ${_header}
      COMMAND ${QT_DBUSXML2CPP_EXECUTABLE} -m -i ${QCA2_INCLUDE_DIR}/QtCrypto -i ${CMAKE_SOURCE_DIR}/daemon/frontend/secret/adaptors/dbustypes.h -p ${_basename} ${_infile}
      DEPENDS ${_infile})

  SET_SOURCE_FILES_PROPERTIES(${_impl} PROPERTIES SKIP_AUTOMOC TRUE)

  QT4_GENERATE_MOC(${_header} ${_moc})

  SET(${_sources} ${${_sources}} ${_impl} ${_header} ${_moc})
  MACRO_ADD_FILE_DEPENDENCIES(${_impl} ${_moc})

ENDMACRO(KSS_ADD_DBUS_INTERFACE)

kss_add_dbus_interface(client_lib_SRCS org.freedesktop.Secret.Collection.xml collection_interface)
kss_add_dbus_interface(client_lib_SRCS org.freedesktop.Secret.Item.xml item_interface )
kss_add_dbus_interface(client_lib_SRCS org.freedesktop.Secret.Prompt.xml prompt_interface )
kss_add_dbus_interface(client_lib_SRCS org.freedesktop.Secret.Service.xml service_interface )
kss_add_dbus_interface(client_lib_SRCS org.freedesktop.Secret.Session.xml session_interface )

kde4_add_library(ksecretserviceclient ${client_lib_SRCS})

TARGET_LINK_LIBRARIES (ksecretserviceclient ${KDE4_KDECORE_LIBS} ${QCA2_LIBRARIES})

SET(ksecretserviceclient_xml 
    org.freedesktop.Secret.Collection.xml
    org.freedesktop.Secret.Item.xml
    org.freedesktop.Secret.Prompt.xml
    org.freedesktop.Secret.Service.xml
    org.freedesktop.Secret.Session.xml
)
INSTALL(FILES 
    ${ksecretserviceclient_xml} 
    DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})

INSTALL(FILES
    ksecretservice.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/ksecretservice COMPONENT Devel 
)

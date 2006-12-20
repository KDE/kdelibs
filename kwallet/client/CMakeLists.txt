

########### next target ###############

set(kwalletclient_LIB_SRCS
kwallet.cc
)

kde4_automoc(${kwalletclient_LIB_SRCS})

set(kwallet_adaptor ${CMAKE_CURRENT_SOURCE_DIR}/../../kio/misc/kwalletd/kwalletdadaptor.h)
set(kwallet_xml ${CMAKE_CURRENT_BINARY_DIR}/org.kde.KWallet.xml)
ADD_CUSTOM_COMMAND(OUTPUT ${kwallet_xml}
       COMMAND ${QT_DBUSCPP2XML_EXECUTABLE} ${kwallet_adaptor} > ${kwallet_xml}
       DEPENDS ${kwallet_adaptor})
QT4_ADD_DBUS_INTERFACE( kwalletclient_LIB_SRCS ${kwallet_xml} kwallet_interface )


# I tried this, with the custom command in kwalletd, didn't work
#QT4_ADD_DBUS_INTERFACE( kwalletclient_LIB_SRCS ${CMAKE_CURRENT_BINARY_DIR}/../../kio/misc/kwalletd/org.kde.KWallet.xml kwallet_interface )

kde4_add_library(kwalletclient SHARED ${kwalletclient_LIB_SRCS})

target_link_libraries(kwalletclient  ${KDE4_KDECORE_LIBS} )

set_target_properties(kwalletclient PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kwalletclient  DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############
install( FILES kwallet.h  DESTINATION  ${INCLUDE_INSTALL_DIR} )


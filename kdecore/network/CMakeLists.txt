
add_subdirectory( tests )



########### next target ###############


########### install files ###############

install( FILES ipv6blacklist  DESTINATION  ${CONFIG_INSTALL_DIR} )
install( FILES kresolver.h    kreverseresolver.h   ksocketaddress.h     ksocketbase.h     ksocketdevice.h   kclientsocketbase.h     kstreamsocket.h   kserversocket.h   kdatagramsocket.h    kmulticastsocketdevice.h   kmulticastsocket.h   knetworkinterface.h     khttpproxysocketdevice.h   ksockssocketdevice.h    kbufferedsocket.h    kiobuffer.h  DESTINATION  ${INCLUDE_INSTALL_DIR} )





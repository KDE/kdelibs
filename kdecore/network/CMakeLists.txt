
add_subdirectory( tests )



########### next target ###############


########### install files ###############

install_files( ${CONFIG_INSTALL_DIR} FILES ipv6blacklist )
install_files( /include FILES kresolver.h    kreverseresolver.h   ksocketaddress.h     ksocketbase.h     ksocketdevice.h   kclientsocketbase.h     kstreamsocket.h   kserversocket.h   kdatagramsocket.h    kmulticastsocketdevice.h   kmulticastsocket.h   knetworkinterface.h     khttpproxysocketdevice.h   ksockssocketdevice.h    kbufferedsocket.h    kiobuffer.h )





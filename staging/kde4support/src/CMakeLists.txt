set(libkde4support_SRCS
    kdecore/ktemporaryfile.cpp
    kdecore/ktempdir.cpp
    kdecore/kmd5.cpp
    kdecore/ksavefile.cpp
    kdecore/k3socks.cpp
    kdecore/k3sockssocketdevice.cpp
    kdecore/k3socketdevice.cpp
    kdecore/k3bufferedsocket.cpp
    kdecore/k3clientsocketbase.cpp
    kdecore/k3resolver.cpp
    kdecore/k3resolvermanager.cpp
    kdecore/k3resolverworkerbase.cpp
    kdecore/k3resolverstandardworkers.cpp
    kdecore/k3reverseresolver.cpp
    kdecore/k3serversocket.cpp
    kdecore/k3socketaddress.cpp
    kdecore/k3socketbase.cpp
    kdecore/k3streamsocket.cpp
    kdecore/k3socketbuffer.cpp
    kdecore/netsupp.cpp

    kdeui/kicon.cpp
    kdeui/knumvalidator.cpp
)

include_directories(
    ${CMAKE_SOURCE_DIR}/kdecore/kernel/ # for KComponentData
    ${CMAKE_BINARY_DIR}/kdecore/
    ${CMAKE_SOURCE_DIR}/kdecore/config/
    ${CMAKE_SOURCE_DIR}/kdecore/io/
    ${CMAKE_SOURCE_DIR}/kdecore/util/
    ${CMAKE_SOURCE_DIR}/kdecore/localization/
    ${kcoreaddons_SOURCE_DIR}/src/randomness # for KRandom
    ${kcoreaddons_SOURCE_DIR}/src/io # for KBackup
    ${kcoreaddons_BINARY_DIR}/src
    ${kconfig_BINARY_DIR}/src # ksharedconfig.h
    ${kconfig_SOURCE_DIR}/src
    ${kwidgets_BINARY_DIR}/src
    ${kwidgets_SOURCE_DIR}/src/icons # kiconloader
)

# Configure checks for network/ but also for netsupp.*
include(ConfigureChecks.cmake)
configure_file(config-network.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-network.h )

add_library(kde4support ${LIBRARY_TYPE} ${libkde4support_SRCS})
generate_export_header(kde4support)

target_link_libraries(kde4support ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTDBUS_LIBRARY} kdecore kcoreaddons kwidgets)
set_target_properties(kde4support PROPERTIES VERSION ${ECM_VERSION_STRING} SOVERSION ${ECM_SOVERSION})

install(TARGETS kde4support EXPORT kdelibsLibraryTargets ${ECM_TARGET_DEFAULT_ARGS})

install(FILES
    kdecore/ktemporaryfile.h
    kdecore/ktempdir.h
    kdecore/kmd5.h
    kdecore/ksavefile.h
    kdecore/k3bufferedsocket.h
    kdecore/k3clientsocketbase.h
    kdecore/k3resolver.h
    kdecore/k3reverseresolver.h
    kdecore/k3serversocket.h
    kdecore/k3socketaddress.h
    kdecore/k3socketbase.h
    kdecore/k3socketdevice.h
    kdecore/k3socks.h
    kdecore/k3sockssocketdevice.h
    kdecore/k3streamsocket.h
    kdeui/kicon.h
    kdeui/knumvalidator.h
    ${CMAKE_CURRENT_BINARY_DIR}/kde4support_export.h
    DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

# kparts/*.h
install(FILES
    kparts/componentfactory.h
    kparts/genericfactory.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kparts COMPONENT Devel
)

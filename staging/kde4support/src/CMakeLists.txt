set(libkde4support_SRCS
    kdecore/klibloader.cpp
    kdecore/ktemporaryfile.cpp
    kdecore/ktempdir.cpp
    kdecore/kmd5.cpp
    kdecore/kmimetype.cpp
    kdecore/kmimetyperepository.cpp
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
    kdecore/kascii.cpp

    kdeui/kapplication.cpp
    kdeui/ksessionmanager.cpp
    kdeui/kfadewidgeteffect.cpp
    kdeui/kicon.cpp
    kdeui/knumvalidator.cpp
    kdeui/karrowbutton.cpp
    kdeui/ksplashscreen.cpp
    kdeui/kdialogbuttonbox.cpp
    kdeui/kstatusbar.cpp
    kdeui/ksystemtrayicon.cpp
    kdeui/kuniqueapplication.cpp
    kdeui/khbox.cpp
    kdeui/kvbox.cpp
)

if (Q_OS_WIN)
    set(libkde4support_SRCS ${libkde4support_SRCS} kernel/kapplication_win.cpp)
endif (Q_OS_WIN)

include_directories(
    ${CMAKE_SOURCE_DIR}/kdecore/kernel/ # for KComponentData
    ${CMAKE_BINARY_DIR}/kdecore/
    ${CMAKE_SOURCE_DIR}/kdecore/config/
    ${CMAKE_SOURCE_DIR}/kdecore/io/
    ${CMAKE_SOURCE_DIR}/kdecore/util/
    ${CMAKE_SOURCE_DIR}/kdecore/localization/
    ${KDE4_KDEUI_INCLUDES}
    ${kcoreaddons_SOURCE_DIR}/src/randomness # for KRandom
    ${kcoreaddons_SOURCE_DIR}/src/io # for KBackup
    ${kcoreaddons_BINARY_DIR}/src
    ${ki18n_SOURCE_DIR}/src
    ${ki18n_BINARY_DIR}/src
    ${kconfig_BINARY_DIR}/src/core # ksharedconfig.h
    ${kconfig_SOURCE_DIR}/src/core
    ${kservice_BINARY_DIR}/src
    ${kservice_SOURCE_DIR}/src/sycoca
    ${kservice_SOURCE_DIR}/src/services
    ${kwidgets_BINARY_DIR}/src
    ${kwidgets_SOURCE_DIR}/src/icons # kiconloader
    ${kwidgets_SOURCE_DIR}/src/utils # kglobalsettings
    ${kwidgets_SOURCE_DIR}/src/widgets # kpushbutton
    ${kauth_BINARY_DIR}/src
    ${kauth_SOURCE_DIR}/src # kpushbutton -> kauthaction
    ${kinterprocesswindowing_BINARY_DIR}/src
    ${kinterprocesswindowing_SOURCE_DIR}/src
)

# Configure checks for network/ but also for netsupp.*
include(ConfigureChecks.cmake)
configure_file(config-network.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-network.h )
configure_file(config-kde4support.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kde4support.h )

configure_file(kdemacros.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdemacros.h )

add_library(kde4support ${libkde4support_SRCS})
generate_export_header(kde4support)

target_link_libraries(kde4support ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTDBUS_LIBRARY} kdecore kcoreaddons kwidgets kauth kdeui ki18n ${X11_LIBRARIES} ${QtMimeTypes_LIBRARY})
set_target_properties(kde4support PROPERTIES VERSION ${ECM_VERSION_STRING} SOVERSION ${ECM_SOVERSION})

install(TARGETS kde4support EXPORT kdelibsLibraryTargets ${ECM_TARGET_DEFAULT_ARGS})

install(FILES
    kdecore/klibloader.h
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
    kdecore/kconfiggroup_kurl.h
    kdecore/kmimetype.h
    kdecore/kgenericfactory.h
    kdecore/kgenericfactory.tcc
    kdecore/ktypelist.h
    kdecore/kascii.h

    kdeui/kapplication.h
    kdeui/ksessionmanager.h
    kdeui/kfadewidgeteffect.h
    kdeui/kicon.h
    kdeui/knumvalidator.h
    kdeui/karrowbutton.h
    kdeui/ksplashscreen.h
    kdeui/kdialogbuttonbox.h
    kdeui/kstatusbar.h
    kdeui/ksystemtrayicon.h
    kdeui/kuniqueapplication.h
    kdeui/khbox.h
    kdeui/kvbox.h
    ${CMAKE_CURRENT_BINARY_DIR}/kde4support_export.h
    ${CMAKE_CURRENT_BINARY_DIR}/kdemacros.h
    DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

# kparts/*.h
install(FILES
    kparts/componentfactory.h
    kparts/genericfactory.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kparts COMPONENT Devel
)

# Generate config-kstandarddirs.h
set( KDELIBSUFF ${LIB_SUFFIX} )
configure_file(config-kstandarddirs.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kstandarddirs.h )

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
    kdecore/qtest_kde.cpp
    kdecore/kstandarddirs.cpp
    kdecore/kglobal.cpp

    kdeui/k3icon.cpp
    kdeui/kaction.cpp
    kdeui/kapplication.cpp
    kdeui/kcolorchoosermode.cpp
    kdeui/kcolordialog.cpp
    kdeui/kcolorhelpers.cpp
    kdeui/kcolorvalueselector.cpp
    kdeui/kdialog.cpp
    kdeui/kdialogqueue.cpp
    kdeui/khuesaturationselect.cpp
    kdeui/ksessionmanager.cpp
    kdeui/kfadewidgeteffect.cpp
    kdeui/kicon.cpp
    kdeui/knumvalidator.cpp
    kdeui/karrowbutton.cpp
    kdeui/ksplashscreen.cpp
    kdeui/kdialogbuttonbox.cpp
    kdeui/kprogressdialog.cpp
    kdeui/kmenubar.cpp
    kdeui/kstatusbar.cpp
    kdeui/ksystemtrayicon.cpp
    kdeui/kuniqueapplication.cpp
    kdeui/khbox.cpp
    kdeui/kvbox.cpp
    kdeui/kmessagebox_queued.cpp
    kdeui/kmessageboxmessagehandler.cpp
    kdeui/kpushbutton.cpp
    kdeui/kundostack.cpp

    kparts/factory.cpp
)

if (Q_OS_WIN)
    set(libkde4support_SRCS ${libkde4support_SRCS} kernel/kapplication_win.cpp)
endif (Q_OS_WIN)

# Configure checks for network/ but also for netsupp.*
include(ConfigureChecks.cmake)
configure_file(config-network.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-network.h )
configure_file(config-kde4support.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kde4support.h )

configure_file(kdemacros.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdemacros.h )

add_library(kde4support ${libkde4support_SRCS})
generate_export_header(kde4support)

set(kde4support_INCLUDES
  "${CMAKE_CURRENT_SOURCE_DIR}/kdecore"
  "${CMAKE_CURRENT_SOURCE_DIR}/kdeui"
  "${CMAKE_SOURCE_DIR}"
  "${CMAKE_CURRENT_SOURCE_DIR}/kparts"
)
target_include_directories(kde4support PUBLIC "$<BUILD_INTERFACE:${kde4support_INCLUDES}>")

target_link_libraries(kde4support Qt5::Widgets ${QT_QTDBUS_LIBRARY} ${QT_QTTEST_LIBRARY} kdecore ${KCoreAddons_LIBRARIES} ${KWidgetsAddons_LIBRARIES} kwidgets ${KAuth_LIBRARIESss} kdeui ki18n kinterprocesswindowing ${X11_LIBRARIES})
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
    kdecore/qtest_kde.h
    kdecore/kstandarddirs.h
    kdecore/kglobal.h

    kdeui/kaction.h
    kdeui/kapplication.h
    kdeui/kcolorchoosermode.h
    kdeui/kcolordialog.h
    kdeui/kcolorvalueselector.h
    kdeui/kdialog.h
    kdeui/khuesaturationselect.h
    kdeui/ksessionmanager.h
    kdeui/kfadewidgeteffect.h
    kdeui/kicon.h
    kdeui/knumvalidator.h
    kdeui/karrowbutton.h
    kdeui/ksplashscreen.h
    kdeui/kdialogbuttonbox.h
    kdeui/kprogressdialog.h
    kdeui/kmenubar.h
    kdeui/kstatusbar.h
    kdeui/ksystemtrayicon.h
    kdeui/kuniqueapplication.h
    kdeui/khbox.h
    kdeui/kvbox.h
    kdeui/kmessagebox_queued.h
    kdeui/kmessageboxmessagehandler.h
    kdeui/kpushbutton.h
    kdeui/kundostack.h
    ${CMAKE_CURRENT_BINARY_DIR}/kde4support_export.h
    ${CMAKE_CURRENT_BINARY_DIR}/kdemacros.h
    DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

install( FILES kdeui/40.colors kdeui/Oxygen.colors kdeui/Web.colors kdeui/Royal.colors kdeui/Rainbow.colors
         DESTINATION ${CONFIG_INSTALL_DIR}/colors)

if (NOT X11_FOUND)
    # install our copy of the rgb.txt named colors list on systems without X11
    install(FILES kdeui/rgb.txt DESTINATION ${DATA_INSTALL_DIR}/kdeui)
endif (NOT X11_FOUND)

# kparts/*.h
install(FILES
    kparts/componentfactory.h
    kparts/genericfactory.h
    kparts/factory.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kparts COMPONENT Devel
)

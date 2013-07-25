find_package(X11)

if (X11_FOUND)
   set (HAVE_X11 1)
else ()
   set (HAVE_X11 0)
endif ()

set(libkde4support_SRCS
    kdecore/kkernel_mac.cpp
    kdecore/kkernel_win.cpp
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
    kdecore/kcmdlineargs.cpp
    kdecore/kurl.cpp

    kdeui/k3icon.cpp
    kdeui/kaction.cpp
    kdeui/kapplication.cpp
    kdeui/kbuttongroup.cpp
    kdeui/kcolorchoosermode.cpp
    kdeui/kcolordialog.cpp
    kdeui/kcolorhelpers.cpp
    kdeui/kcolorvalueselector.cpp
    kdeui/kdialog.cpp
    kdeui/kdialogqueue.cpp
    kdeui/kfontcombobox.cpp
    kdeui/fonthelpers.cpp
    kdeui/kfontdialog.cpp
    kdeui/khuesaturationselect.cpp
    kdeui/ksessionmanager.cpp
    kdeui/kfadewidgeteffect.cpp
    kdeui/kicon.cpp
    kdeui/kinputdialog.cpp
    kdeui/knumvalidator.cpp
    kdeui/karrowbutton.cpp
    kdeui/ksplashscreen.cpp
    kdeui/kstringvalidator.cpp
    kdeui/kdialogbuttonbox.cpp
    kdeui/kpixmapcache.cpp
    kdeui/kprogressdialog.cpp
    kdeui/kmenu.cpp
    kdeui/kmenubar.cpp
    kdeui/kshortcut.cpp
    kdeui/kstatusbar.cpp
    kdeui/ksystemtrayicon.cpp
    kdeui/ktabbar.cpp
    kdeui/ktabwidget.cpp
    kdeui/kuniqueapplication.cpp
    kdeui/khbox.cpp
    kdeui/kvbox.cpp
    kdeui/kmessagebox_queued.cpp
    kdeui/kmessageboxmessagehandler.cpp
    kdeui/kpushbutton.cpp
    kdeui/kundostack.cpp
    kdeui/krestrictedline.cpp
    kdeui/klistwidget.cpp
    kdeui/ktextbrowser.cpp
    kdeui/keditlistbox.cpp
    kdeui/knuminput.cpp

    kio/kfilemetainfowidget.cpp
    kio/kdiskfreespace.cpp
    kio/metainfojob.cpp
    kio/passworddialog.cpp
    kio/kmimetyperesolver.cpp
    kio/kdatatool.cpp

    kparts/factory.cpp
)

if (X11_FOUND)
   set(libkde4support_SRCS ${libkde4support_SRCS} kdeui/kxerrorhandler.cpp )
endif ()

if (WIN32)
    set(libkde4support_SRCS ${libkde4support_SRCS} kdecore/klockfile_win.cpp)
else()
    set(libkde4support_SRCS ${libkde4support_SRCS} kdecore/klockfile_unix.cpp)
endif ()

# Configure checks for network/ but also for netsupp.*
include(ConfigureChecks.cmake)
configure_file(config-network.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-network.h )
configure_file(config-kde4support.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kde4support.h )
check_include_files(sys/stat.h HAVE_SYS_STAT_H)
check_include_files(sys/time.h HAVE_SYS_TIME_H)
configure_file(kdecore/config-klockfile.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-klockfile.h)
# Generate config-kstandarddirs.h
configure_file(config-kstandarddirs.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kstandarddirs.h )

configure_file(kdemacros.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdemacros.h )

add_library(KDE4Support ${libkde4support_SRCS})
generate_export_header(KDE4Support)

target_include_directories(KDE4Support PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/../../..")

set(kde4support_INCLUDES
  "${CMAKE_CURRENT_SOURCE_DIR}/kdecore"
  "${CMAKE_CURRENT_SOURCE_DIR}/kdeui"
  "${CMAKE_CURRENT_SOURCE_DIR}/kio"
  "${CMAKE_SOURCE_DIR}"
  "${CMAKE_CURRENT_SOURCE_DIR}/kparts"
)
target_include_directories(KDE4Support PUBLIC "$<BUILD_INTERFACE:${kde4support_INCLUDES}>")

target_link_libraries(KDE4Support Qt5::Widgets Qt5::DBus Qt5::Test kdecore ${KCoreAddons_LIBRARIES} KCrash ${KWidgetsAddons_LIBRARIES} KWidgets kdeui kio KI18n KInterProcessWindowing ${X11_LIBRARIES} KCompletion KDE4Attic)

set_target_properties(KDE4Support PROPERTIES VERSION ${KDE4SUPPORT_VERSION_STRING}
                                  SOVERSION ${KDE4SUPPORT_SOVERSION} )

install(TARGETS KDE4Support
        EXPORT kdelibsLibraryTargets # KDE4SupportTargets <- cannot be moved to separate export set as long as kde4support depends on kdelibs and kdelibs depends on kde4support
        ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    kdecore/kkernel_mac.h
    kdecore/kkernel_win.h
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
    kdecore/kcmdlineargs.h
    kdecore/klockfile.h
    kdecore/kurl.h

    kio/kdiskfreespace.h
    kio/kdatatool.h
    kio/kmimetyperesolver.h

    kdeui/kaccelgen.h
    kdeui/kaction.h
    kdeui/kapplication.h
    kdeui/kbuttongroup.h
    kdeui/kcolorchoosermode.h
    kdeui/kcolordialog.h
    kdeui/kcolorvalueselector.h
    kdeui/kdialog.h
    kdeui/khuesaturationselect.h
    kdeui/ksessionmanager.h
    kdeui/kfadewidgeteffect.h
    kdeui/kfontdialog.h
    kdeui/kicon.h
    kdeui/kinputdialog.h
    kdeui/knumvalidator.h
    kdeui/karrowbutton.h
    kdeui/ksplashscreen.h
    kdeui/ktabbar.h
    kdeui/ktabwidget.h
    kdeui/kdialogbuttonbox.h
    kdeui/kpixmapcache.h
    kdeui/kprogressdialog.h
    kdeui/kmenu.h
    kdeui/kmenubar.h
    kdeui/kshortcut.h
    kdeui/kstatusbar.h
    kdeui/kstringvalidator.h
    kdeui/ksystemtrayicon.h
    kdeui/kuniqueapplication.h
    kdeui/khbox.h
    kdeui/kvbox.h
    kdeui/kmessagebox_queued.h
    kdeui/kmessageboxmessagehandler.h
    kdeui/kpushbutton.h
    kdeui/kundostack.h
    kdeui/krestrictedline.h
    kdeui/klistwidget.h
    kdeui/ktextbrowser.h
    kdeui/keditlistbox.h
    kdeui/knuminput.h
    kdeui/kviewstatesaver.h
    kdeui/kxerrorhandler.h
    ${CMAKE_CURRENT_BINARY_DIR}/kde4support_export.h
    ${CMAKE_CURRENT_BINARY_DIR}/kdemacros.h
    DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

install( FILES kdeui/40.colors kdeui/Oxygen.colors kdeui/Web.colors kdeui/Royal.colors kdeui/Rainbow.colors
         DESTINATION ${CONFIG_INSTALL_DIR}/colors)

if (NOT X11_FOUND)
    # install our copy of the rgb.txt named colors list on systems without X11
    install(FILES kdeui/rgb.txt DESTINATION ${DATA_INSTALL_DIR}/kdeui)
endif ()

# kio/*.h
install(FILES
    kio/metainfojob.h
    kio/passworddialog.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kio COMPONENT Devel
)

# kparts/*.h
install(FILES
    kparts/componentfactory.h
    kparts/genericfactory.h
    kparts/factory.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kparts COMPONENT Devel
)

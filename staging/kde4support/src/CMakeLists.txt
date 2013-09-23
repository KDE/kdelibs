find_package(X11)

if (X11_FOUND)
   set (HAVE_X11 1)
else ()
   set (HAVE_X11 0)
endif ()

if(NOT HAVE_NEPOMUK)
   set(KIO_NO_NEPOMUK TRUE)
endif(NOT HAVE_NEPOMUK)

if(NOT Strigi_FOUND)
    set(KIO_NO_STRIGI TRUE)
else()
    include_directories(${STRIGI_INCLUDE_DIR} ${STRIGI_INCLUDE_DIRS} ${LIBSTREAMANALYZER_INCLUDE_DIRS})
    add_subdirectory( kio/dummyanalyzers )
    set(KIO_EXTRA_LIBS ${KIO_EXTRA_LIBS} ${STRIGI_STREAMANALYZER_LIBRARY} ${STRIGI_STREAMS_LIBRARY})
endif(NOT Strigi_FOUND)

if(NOT KIO_NO_NEPOMUK)
    include_directories(
       ${SOPRANO_INCLUDE_DIR}
    )
endif(NOT KIO_NO_NEPOMUK)


find_package(OpenSSL)
set_package_properties(OpenSSL PROPERTIES DESCRIPTION "Support for secure network communications (SSL and TLS)"
                       URL "http://openssl.org"
                       TYPE RECOMMENDED
                       PURPOSE "KDE uses OpenSSL for the bulk of secure communications, including secure web browsing via HTTPS"
                      )
if(OPENSSL_FOUND)
   set(KSSL_HAVE_SSL 1)
   include_directories(${OPENSSL_INCLUDE_DIR})
endif(OPENSSL_FOUND)

add_subdirectory( kioslave )
add_subdirectory( kssl/kssl )
add_subdirectory( kssl/kcm )

set(libkde4support_SRCS
    kdecore/k4aboutdata.cpp
    kdecore/kcomponentdata.cpp
    kdecore/kdebug.cpp
    kdecore/kdebugdbusiface.cpp
    kdecore/kdeversion.cpp
    kdecore/kkernel_mac.cpp
    kdecore/kkernel_win.cpp
    kdecore/klibloader.cpp
    kdecore/ktemporaryfile.cpp
    kdecore/ktempdir.cpp
    kdecore/kmd5.cpp
    kdecore/kmimetype.cpp
    kdecore/kmimetyperepository.cpp
    kdecore/ksavefile.cpp
    kdecore/ksocketfactory.cpp
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
    kdeui/k4style.cpp
    kdeui/kaction.cpp
    kdeui/kapplication.cpp
    kdeui/kbuttongroup.cpp
    kdeui/kcolorchoosermode.cpp
    kdeui/kcolordialog.cpp
    kdeui/kcolorhelpers.cpp
    kdeui/kcolorvalueselector.cpp
    kdeui/kdeuiwidgetsproxystyle.cpp
    kdeui/kdialog.cpp
    kdeui/kdialogqueue.cpp
    kdeui/kfontcombobox.cpp
    kdeui/fonthelpers.cpp
    kdeui/kfontdialog.cpp
    kdeui/khuesaturationselect.cpp
    kdeui/kpassivepopupmessagehandler.cpp
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
    kdeui/kglobalsettings.cpp

    kio/kfilemetainfowidget.cpp
    kio/kdiskfreespace.cpp
    kio/metainfojob.cpp
    kssl/ksslx509v3.cpp
    kssl/kssl.cpp
    kssl/ksslutils.cpp
    kssl/ksslcertchain.cpp
    kssl/ksslcertificate.cpp
    kio/kfileshare.cpp
    kio/kfileitemactionplugin.cpp
    kio/passworddialog.cpp
    kio/kmimetyperesolver.cpp
    kio/kdatatool.cpp
    kio/kfilemetadataconfigurationwidget.cpp
    kio/kfilemetadatawidget.cpp
    kio/kfilemetainfowidget.cpp
    kio/kmetaprops.cpp
    kio/knfotranslator.cpp
    kio/kfilemetainfo.cpp
    kio/kfilemetainfoitem.cpp
    kio/kcommentwidget.cpp
    kio/predicateproperties.cpp
    kio/kfilewriteplugin.cpp
#     kio/kpreviewprops.cpp
    kio/kscan.cpp
    kio/netaccess.cpp

    kparts/factory.cpp
)

if (NOT KIO_NO_NEPOMUK)
    set(libkde4support_SRCS ${libkde4support_SRCS}
        kio/kfilemetadatareader.cpp
        kio/kfilemetadatareaderprocess.cpp
        kio/kfilemetadataprovider.cpp
    )
endif ()

if (X11_FOUND)
   set(libkde4support_SRCS ${libkde4support_SRCS} kdeui/kxerrorhandler.cpp )
endif ()

if (WIN32)
    set(libkde4support_SRCS ${libkde4support_SRCS} kdecore/klockfile_win.cpp util/kde_file_win.cpp)
else()
    set(libkde4support_SRCS ${libkde4support_SRCS} kdecore/klockfile_unix.cpp)
endif ()

# Configure checks for io/
include(kdecore/ConfigureChecks.cmake)
configure_file(kdecore/config-io.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-io.h)

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

# Generate kdeversion.h
configure_file(kdecore/kdeversion.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdeversion.h )

target_include_directories(KDE4Support PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/../../..")

set(kde4support_INCLUDES
  "${CMAKE_CURRENT_SOURCE_DIR}/kdecore"
  "${CMAKE_CURRENT_SOURCE_DIR}/kdeui"
  "${CMAKE_CURRENT_SOURCE_DIR}/kio"
  "${CMAKE_CURRENT_SOURCE_DIR}/kssl"
  "${CMAKE_CURRENT_BINARY_DIR}/kssl" # ksslconfig.h
  "${CMAKE_CURRENT_SOURCE_DIR}/kparts"
  "${CMAKE_SOURCE_DIR}"
)
target_include_directories(KDE4Support PUBLIC "$<BUILD_INTERFACE:${kde4support_INCLUDES}>")

target_link_libraries(KDE4Support
   Qt5::Widgets
   Qt5::DBus
   Qt5::Test
   Qt5::X11Extras
   Qt5::Svg
   ${KCoreAddons_LIBRARIES}
   KCrash
   ${KWidgetsAddons_LIBRARIES}
   KConfigWidgets
   KIOCore
   kio
   ${KIO_EXTRA_LIBS}
   KI18n
   KInterProcessWindowing
   ${X11_LIBRARIES}
   KCompletion
   KDE4Attic
   KNotifications
   KIconThemes
   KWindowSystem
   KGuiAddons
   KTextWidgets
)

# This flag is needed in order to be able to support files > 2GB even on
# 32bit platforms. The default is to use the non-64bit aware syscalls on
# 32bit platforms, which makes every application to SIGXFSZ (which is
# equivalent to a crash) when they see or touch a file > 2GB.
check_cxx_source_compiles("
#include <sys/types.h>
/* Check that off_t can represent 2**63 - 1 correctly.
We can't simply define LARGE_OFF_T to be 9223372036854775807,
since some C++ compilers masquerading as C compilers
incorrectly reject 9223372036854775807.  */
#define LARGE_OFF_T (((off_t) 1 << 62) - 1 + ((off_t) 1 << 62))
int off_t_is_large[(LARGE_OFF_T % 2147483629 == 721 && LARGE_OFF_T % 2147483647 == 1) ? 1 : -1];
int main() { return 0; }
" _OFFT_IS_64BIT)

if (NOT _OFFT_IS_64BIT)
  target_compile_definitions(KDE4Support PUBLIC -D_FILE_OFFSET_BITS=64)
endif ()


if(NOT KIO_NO_NEPOMUK)
  target_link_libraries(KDE4Support ${KDE4_NEPOMUK_LIBRARY} nepomukquery nepomukutils)
endif(NOT KIO_NO_NEPOMUK)

if(NOT KIO_NO_NEPOMUK)
    add_executable(kfilemetadatareader kio/kfilemetadatareaderprocess.cpp)
    target_link_libraries(kfilemetadatareader ${KDE4_NEPOMUK_LIBRARY} nepomukutils ${KCoreAddons_LIBRARIES} KI18n KDE4Support)
    install(TARGETS kfilemetadatareader ${INSTALL_TARGETS_DEFAULT_ARGS})
endif(NOT KIO_NO_NEPOMUK)

set_target_properties(KDE4Support PROPERTIES VERSION ${KDE4SUPPORT_VERSION_STRING}
                                  SOVERSION ${KDE4SUPPORT_SOVERSION} )

install(TARGETS KDE4Support
        EXPORT kdelibsLibraryTargets # KDE4SupportTargets <- cannot be moved to separate export set as long as kde4support depends on kdelibs and kdelibs depends on kde4support
        ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    kdecore/k4aboutdata.h
    kdecore/kcomponentdata.h
    kdecore/kdebug.h
    kdecore/kde_file.h
    kdecore/klibloader.h
    kdecore/ktemporaryfile.h
    kdecore/ktempdir.h
    kdecore/kmd5.h
    kdecore/ksavefile.h
    kdecore/ksocketfactory.h
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
    kdecore/ksharedptr.h

    kio/kdiskfreespace.h
    kio/kdatatool.h
    kio/kmimetyperesolver.h
    kio/kfilemetadataconfigurationwidget.h
    kio/kfilemetadatawidget.h
    kio/kfilemetainfoitem.h
    kio/kfilemetainfowidget.h
    kio/kfilemetainfo.h
    kio/predicateproperties.h
    kio/kfilewriteplugin.h
    kio/kfileshare.h
    kio/kfileitemactionplugin.h
    kio/kscan.h
    kssl/ksslcertificate.h
    kssl/ksslx509v3.h
    kssl/kssl.h
    kssl/ksslutils.h
    kssl/ksslcertchain.h

    kdeui/k4style.h
    kdeui/kaccelgen.h
    kdeui/kaction.h
    kdeui/kapplication.h
    kdeui/kbuttongroup.h
    kdeui/kcolorchoosermode.h
    kdeui/kcolordialog.h
    kdeui/kcolorvalueselector.h
    kdeui/kdialog.h
    kdeui/khuesaturationselect.h
    kdeui/kpassivepopupmessagehandler.h
    kdeui/ksessionmanager.h
    kdeui/kfadewidgeteffect.h
    kdeui/kfontcombobox.h
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
    kdeui/kglobalsettings.h
    ${CMAKE_CURRENT_BINARY_DIR}/kde4support_export.h
    ${CMAKE_CURRENT_BINARY_DIR}/kdemacros.h
    ${CMAKE_CURRENT_BINARY_DIR}/kdeversion.h
    DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

configure_file(kssl/ksslconfig.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kssl/ksslconfig.h )
set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_CURRENT_BINARY_DIR}/kssl/ksslconfig.h )

install( FILES kdebug.areas kdebugrc DESTINATION ${CONFIG_INSTALL_DIR} )

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
    kio/netaccess.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kio COMPONENT Devel
)

# kparts/*.h
install(FILES
    kparts/componentfactory.h
    kparts/genericfactory.h
    kparts/factory.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kparts COMPONENT Devel
)

install(FILES
    kio/kfilewrite.desktop
    DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

install( FILES
    KDECoreMacros.cmake
    KDEUIMacros.cmake
    kde4uic.cmake
    DESTINATION ${CMAKECONFIG_INSTALL_DIR} COMPONENT Devel )

install( FILES
    kio/kscan.desktop
    DESTINATION  ${SERVICETYPES_INSTALL_DIR} )

project(kdecore)

# Generate config-kstandarddirs.h
configure_file(config-kstandarddirs.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kstandarddirs.h )

# Generate kdefakes.h
configure_file(kdefakes.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdefakes.h )

# Generate kdeversion.h
configure_file(util/kdeversion.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdeversion.h )

# Configure checks for kdirwatch
find_package(FAM)
macro_bool_to_01(FAM_FOUND HAVE_FAM)

check_include_files(sys/inotify.h SYS_INOTIFY_H_FOUND)
macro_bool_to_01(SYS_INOTIFY_H_FOUND HAVE_SYS_INOTIFY_H)

# Configure checks for network/ but also for netsupp.*
include(network/ConfigureChecks.cmake)
configure_file(network/config-network.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/network/config-network.h )
include_directories( ${CMAKE_CURRENT_BINARY_DIR}/network )

# Configure checks for date/
include(date/ConfigureChecks.cmake)
configure_file(date/config-date.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/date/config-date.h)
include_directories( ${CMAKE_CURRENT_BINARY_DIR}/date )

# Configure checks for localization
configure_file(localization/config-localization.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-localization.h)


include_directories( ${KDE4_KDECORE_INCLUDES} )
include_directories( ${ZLIB_INCLUDE_DIR} )
include_directories( ${QT_INCLUDES} )
include_directories( ${CMAKE_SOURCE_DIR}/libqtmimetypes/include/QtMimeTypes )
include_directories( ${CMAKE_SOURCE_DIR}/staging/kcoreaddons/src/jobs )
include_directories( ${CMAKE_SOURCE_DIR}/staging/kcoreaddons/src/io )
include_directories( ${CMAKE_SOURCE_DIR}/staging/kcoreaddons/src/text )
include_directories( ${CMAKE_SOURCE_DIR}/staging/kcoreaddons/src/kernel )
include_directories( ${CMAKE_SOURCE_DIR}/staging/kcoreaddons/src/randomness )
include_directories( ${CMAKE_BINARY_DIR}/staging/kcoreaddons/src/ )

if (KDE_PLATFORM_FEATURE_DISABLE_DEPRECATED)
   set(KDECORE_NO_KDE3SUPPORT TRUE)
endif(KDE_PLATFORM_FEATURE_DISABLE_DEPRECATED)

# kdecore_OPTIONAL_SRCS is used to collect source files
# which are not always compiled into kdecore
# if it's used, *always* append to it
set(kdecore_OPTIONAL_SRCS)
# same for optional libs
set(kdecore_OPTIONAL_LIBS)

add_definitions(-DQT_NO_CAST_FROM_ASCII)

if(WIN32)
   set(kdecore_OPTIONAL_LIBS ${kdecore_OPTIONAL_LIBS} ${KDEWIN_LIBRARIES})
endif(WIN32)

# Needed for the kdatetime test in release mode
if(KDE4_BUILD_TESTS)
   add_definitions(-DCOMPILING_TESTS)
endif(KDE4_BUILD_TESTS)

# compile Gettext support if available
if(LIBINTL_FOUND)
  include_directories(${LIBINTL_INCLUDE_DIR})
  set(kdecore_OPTIONAL_LIBS ${kdecore_OPTIONAL_LIBS} ${LIBINTL_LIBRARIES})
endif(LIBINTL_FOUND)

if (HAVE_RESOLV_LIBRARY)
  set(kdecore_OPTIONAL_LIBS ${kdecore_OPTIONAL_LIBS} resolv)
endif (HAVE_RESOLV_LIBRARY)

if (APPLE)
   set(kdecore_OPTIONAL_SRCS ${kdecore_OPTIONAL_SRCS}
      kernel/kkernel_mac.cpp
      localization/klocale_mac.cpp
   )

   set(kdecore_OPTIONAL_LIBS ${kdecore_OPTIONAL_LIBS} ${CARBON_LIBRARY})
endif (APPLE)

if (WIN32)
   LIST(APPEND kdecore_OPTIONAL_SRCS
      kernel/kkernel_win.cpp
      kernel/ktoolinvocation_win.cpp
      localization/klocale_win.cpp
      network/klocalsocket_win.cpp
      util/kde_file_win.cpp
      util/kshell_win.cpp
      date/ktimezone_win.cpp
   )
   if(NOT WINCE)
       LIST(APPEND kdecore_OPTIONAL_SRCS
          util/kuser_win.cpp
       )
   else(NOT WINCE)
       LIST(APPEND kdecore_OPTIONAL_SRCS
          util/kuser_wince.cpp
       )
   endif(NOT WINCE)
   if(NOT MSVC)
      LIST(APPEND kdecore_OPTIONAL_SRCS
         network/netsupp.cpp
      )
   endif(NOT MSVC)
endif (WIN32)


if (UNIX)
   set(kdecore_OPTIONAL_SRCS ${kdecore_OPTIONAL_SRCS}
      kernel/ktoolinvocation_x11.cpp
      localization/klocale_unix.cpp
      network/klocalsocket_unix.cpp
      network/netsupp.cpp
      util/kshell_unix.cpp
      util/kuser_unix.cpp
      fakes.c
   )
   if (NOT KDECORE_NO_KDE3SUPPORT)
      set(kdecore_OPTIONAL_SRCS ${kdecore_OPTIONAL_SRCS}
         network/k3socks.cpp
         network/k3sockssocketdevice.cpp
      )
   endif (NOT KDECORE_NO_KDE3SUPPORT)
endif (UNIX)

add_subdirectory( kconfig_compiler )
add_subdirectory( tests )
if ( NOT WINCE )
    add_subdirectory( network/kssld )
endif ( NOT WINCE )

########### next target ###############

if (UNIX)

   kde4_add_library(kdefakes ${LIBRARY_TYPE} fakes.c)
   set_target_properties(kdefakes PROPERTIES VERSION ${KDE_NON_GENERIC_LIB_VERSION} SOVERSION ${KDE_NON_GENERIC_LIB_SOVERSION})
   if (NOT HAVE_TRUNC)
       target_link_libraries(kdefakes  m )
   endif (NOT HAVE_TRUNC)
   install(TARGETS kdefakes EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

endif (UNIX)



########### next target ###############

set(kdecore_LIB_SRCS
   config/kconfig.cpp
   config/kconfigbase.cpp
   config/kconfiggroup.cpp
   config/kconfigbackend.cpp
   config/kconfigini.cpp
   config/kdesktopfile.cpp
   config/ksharedconfig.cpp
   config/kcoreconfigskeleton.cpp
   date/kcalendarera.cpp
   date/kcalendarsystem.cpp
   date/kcalendarsystemcoptic.cpp
   date/kcalendarsystemethiopian.cpp
   date/kcalendarsystemqdate.cpp
   date/kcalendarsystemgregorian.cpp
   date/kcalendarsystemislamiccivil.cpp
   date/kcalendarsystemhebrew.cpp
   date/kcalendarsystemindiannational.cpp
   date/kcalendarsystemjalali.cpp
   date/kcalendarsystemjapanese.cpp
   date/kcalendarsystemjulian.cpp
   date/kcalendarsystemminguo.cpp
   date/kcalendarsystemthai.cpp
   date/kdatetime.cpp
   date/kdatetimeformatter.cpp
   date/kdatetimeparser.cpp
   date/kdayperiod.cpp
   date/klocalizeddate.cpp
   date/ktimezone.cpp
   date/ksystemtimezone.cpp
   date/ktzfiletimezone.cpp
   io/kdebug.cpp
   io/kdebugdbusiface.cpp
   io/kmessage.cpp
   io/kmountpoint.cpp
   io/kprocess.cpp
   kernel/kauthorized.cpp
   kernel/kglobal.cpp
   kernel/kcomponentdata.cpp
   kernel/kstandarddirs.cpp
   kernel/ktoolinvocation.cpp
   services/kmimetypefactory.cpp
   services/kmimetypetrader.cpp
   services/kmimetype.cpp
   services/kmimetyperepository.cpp
   services/kservice.cpp
   services/kserviceaction.cpp
   services/kservicefactory.cpp
   services/kservicegroup.cpp
   services/kservicegroupfactory.cpp
   services/kserviceoffer.cpp
   services/kservicetype.cpp
   services/kservicetypefactory.cpp
   services/kservicetypeprofile.cpp
   services/kservicetypetrader.cpp
   services/ktraderparse.cpp
   services/ktraderparsetree.cpp
   services/yacc.c
   services/lex.c
   services/kplugininfo.cpp
)

if (NOT KDECORE_NO_KDE3SUPPORT)
   set(kdecore_LIB_SRCS ${kdecore_LIB_SRCS}
      network/k3socketdevice.cpp
      network/k3bufferedsocket.cpp
      network/k3clientsocketbase.cpp
      network/k3resolver.cpp
      network/k3resolvermanager.cpp
      network/k3resolverworkerbase.cpp
      network/k3resolverstandardworkers.cpp
      network/k3reverseresolver.cpp
      network/k3serversocket.cpp
      network/k3socketaddress.cpp
      network/k3socketbase.cpp
      network/k3streamsocket.cpp
      network/k3socketbuffer.cpp
   )
endif (NOT KDECORE_NO_KDE3SUPPORT)

set(kdecore_LIB_SRCS ${kdecore_LIB_SRCS}
   network/klocalsocket.cpp
   network/ksocketfactory.cpp
   network/ksslcertificatemanager.cpp
   network/ktcpsocket.cpp
   localization/kcatalog.cpp
   localization/kcurrencycode.cpp
   localization/klocale.cpp
   localization/klocale_kde.cpp
   localization/klocalizedstring.cpp
   localization/kuitsemantics.cpp
   localization/kuitformats.cpp
   localization/common_helpers.cpp
   sycoca/ksycoca.cpp
   sycoca/ksycocadict.cpp
   sycoca/ksycocaentry.cpp
   sycoca/ksycocafactory.cpp
   sycoca/kprotocolinfo.cpp
   sycoca/kprotocolinfofactory.cpp
   sycoca/kmemfile.cpp
   text/kascii.cpp
   util/kallocator.cpp
   util/kautostart.cpp
   util/kdedmodule.cpp
   util/kdeversion.cpp
   util/klauncher_iface.cpp
   util/klibrary.cpp
   util/kpluginfactory.cpp
   util/kpluginloader.cpp
   util/kshell.cpp
   util/qtest_kde.cpp
   ${kdecore_OPTIONAL_SRCS}
)

if(NOT KDE_NO_DEPRECATED)
    set(kdecore_LIB_SRCS ${kdecore_LIB_SRCS}
       util/klibloader.cpp
    )
endif(NOT KDE_NO_DEPRECATED)


if (NOT Q_WS_X11 AND NOT Q_WS_QWS)
   add_definitions(-DNO_DISPLAY)
endif (NOT Q_WS_X11 AND NOT Q_WS_QWS)

kde4_add_library(kdecore ${LIBRARY_TYPE} ${kdecore_LIB_SRCS})
generate_export_header(kdecore)

target_link_libraries(kdecore
  LINK_PUBLIC
    ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTDBUS_LIBRARY} kcoreaddons
  LINK_PRIVATE
    QtMimeTypes ${QT_QTXML_LIBRARY} ${kdecore_OPTIONAL_LIBS}
)

if(WINCE)
  target_link_libraries(kdecore ${WCECOMPAT_LIBRARIES} Ceshell.lib)
endif(WINCE)

set(SYS_INOTIFY_H_FOUND 0)
if(SYS_INOTIFY_H_FOUND)
   macro_log_feature(FAM_FOUND "FAM" "File alteration notification support via a separate service" "http://oss.sgi.com/projects/fam" FALSE "" "You have file alteration notification support built into your kernel, however you might consider installing FAM as it also supports NFS.")
else(SYS_INOTIFY_H_FOUND)
   macro_log_feature(FAM_FOUND "FAM" "File alteration notification support via a separate service" "http://oss.sgi.com/projects/fam" FALSE "" "Provides file alteration notification facilities using a separate service.")
endif(SYS_INOTIFY_H_FOUND)

if(FAM_FOUND)
   include_directories(${FAM_INCLUDE_DIR})
   target_link_libraries(kdecore ${FAM_LIBRARIES})
endif(FAM_FOUND)

if(WIN32)
  set(kdecore_LINK_INTERFACE_LIBRARIES  ${QT_QTDBUS_LIBRARY} ${QT_QTCORE_LIBRARY} ${KDEWIN_LIBRARIES})
else(WIN32)
  if(APPLE)
    set(kdecore_LINK_INTERFACE_LIBRARIES ${QT_QTDBUS_LIBRARY} ${QT_QTCORE_LIBRARY} ${CARBON_LIBRARY})
  else(APPLE)
    set(kdecore_LINK_INTERFACE_LIBRARIES ${QT_QTDBUS_LIBRARY} ${QT_QTCORE_LIBRARY} )
  endif(APPLE)
endif(WIN32)

target_link_libraries(kdecore LINK_INTERFACE_LIBRARIES ${kdecore_LINK_INTERFACE_LIBRARIES} )

set_target_properties(kdecore PROPERTIES
                              VERSION ${KDE_NON_GENERIC_LIB_VERSION}
                              SOVERSION ${KDE_NON_GENERIC_LIB_SOVERSION}
                     )


install(TARGETS kdecore EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

########### next target ###############

configure_file(localization/all_languages.desktop ${CMAKE_CURRENT_BINARY_DIR}/all_languages @ONLY)

kde4_add_executable(kde4-config NOGUI kde-config.cpp )

target_link_libraries(kde4-config ${KDE4_KDECORE_LIBS} kcoreaddons)

install(TARGETS kde4-config ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### next target ###############

if(NOT WINCE)
    # kjs hash stuff for transcript plugin
    set( CREATE_HASH_TABLE ${CMAKE_SOURCE_DIR}/kjs/create_hash_table )

    add_custom_command(
       OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/ktranscript.lut.h
       COMMAND ${PERL_EXECUTABLE} ${CREATE_HASH_TABLE}
               ${CMAKE_CURRENT_SOURCE_DIR}/localization/ktranscript.cpp
               > ${CMAKE_CURRENT_BINARY_DIR}/ktranscript.lut.h
       DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/localization/ktranscript.cpp
    )

    # the transcript plugin
    set( ktranscript_LIB_SRCS
       localization/ktranscript.cpp
       localization/common_helpers.cpp
    )

    kde4_add_plugin( ktranscript ${ktranscript_LIB_SRCS} )

    target_link_libraries( ktranscript kjs ${QT_QTCORE_LIBRARY} )

    macro_add_file_dependencies(${CMAKE_CURRENT_SOURCE_DIR}/localization/ktranscript.cpp ${CMAKE_CURRENT_BINARY_DIR}/ktranscript.lut.h)

    install( TARGETS ktranscript DESTINATION ${PLUGIN_INSTALL_DIR} )

endif(NOT WINCE)

########### install files ###############
install( FILES ${CMAKE_CURRENT_BINARY_DIR}/all_languages DESTINATION ${LOCALE_INSTALL_DIR}  )
install( FILES localization/entry.desktop  DESTINATION  ${LOCALE_INSTALL_DIR}/en_US )
install( FILES kdebug.areas kdebugrc DESTINATION ${CONFIG_INSTALL_DIR} )
install( FILES
   ${CMAKE_CURRENT_BINARY_DIR}/kdecore_export.h
   config/conversion_check.h
   config/kconfig.h
   #config/kconfigbackend.h re-enable post-API review and implementation (4.2?)
   config/kconfigbase.h
   config/kconfiggroup.h
   config/kdesktopfile.h
   config/ksharedconfig.h
   config/kcoreconfigskeleton.h
   date/kcalendarsystem.h
   date/kdatetime.h
   date/klocalizeddate.h
   date/ksystemtimezone.h
   date/ktimezone.h
   date/ktzfiletimezone.h
   io/kdebug.h
   io/kmessage.h
   io/kprocess.h
   io/kmountpoint.h
   kernel/kauthorized.h
   kernel/kglobal.h
   kernel/kcomponentdata.h
   kernel/kstandarddirs.h
   kernel/ktoolinvocation.h
   DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

if (NOT KDECORE_NO_KDE3SUPPORT)
   install(FILES
      network/k3bufferedsocket.h
      network/k3clientsocketbase.h
      network/k3resolver.h
      network/k3reverseresolver.h
      network/k3serversocket.h
      network/k3socketaddress.h
      network/k3socketbase.h
      network/k3socketdevice.h
      network/k3socks.h
      network/k3sockssocketdevice.h
      network/k3streamsocket.h
      DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
   )
endif (NOT KDECORE_NO_KDE3SUPPORT)

install(FILES
   network/klocalsocket.h
   network/ksocketfactory.h
   network/ktcpsocket.h
   services/kmimetype.h
   services/kmimetypetrader.h
   services/kservice.h
   services/kserviceaction.h
   services/kservicegroup.h
   #services/kservicefactory.h: do not install, internal API
   services/kservicetype.h
   #services/kservicetypefactory.h: do not install, internal API
   services/kservicetypeprofile.h
   services/kservicetypetrader.h
   #services/kserviceoffer.h: do not install, internal API
   services/kplugininfo.h
   localization/kcurrencycode.h
   localization/klocale.h
   localization/klocalizedstring.h
   sycoca/kprotocolinfo.h
   sycoca/ksycoca.h
   sycoca/ksycocaentry.h
   sycoca/ksycocatype.h
   text/kascii.h
   util/kallocator.h
   util/kautostart.h
   util/kde_file.h
   util/kdedmodule.h
   util/kgenericfactory.h
   util/kgenericfactory.tcc
   util/klauncher_iface.h
   util/klibrary.h
   util/klibloader.h
   util/kexportplugin.h
   util/kpluginfactory.h
   util/kpluginloader.h
   util/kshell.h
   util/ksharedptr.h
   util/ksortablelist.h
   util/ktypelist.h
   util/ktypelistutils.h
   util/ktypetraits.h
   util/kuser.h
   util/qtest_kde.h
   ${CMAKE_CURRENT_BINARY_DIR}/kdefakes.h
   ${CMAKE_CURRENT_BINARY_DIR}/kdeversion.h
   DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

install( FILES
   config/kconfigbackend.desktop
   services/kplugininfo.desktop
   DESTINATION  ${SERVICETYPES_INSTALL_DIR}
)

install( FILES
    KDECoreMacros.cmake
    DESTINATION ${DATA_INSTALL_DIR}/cmake/modules COMPONENT Devel )

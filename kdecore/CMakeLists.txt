project(kdecore)

# Generate config-kstandarddirs.h
configure_file(config-kstandarddirs.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kstandarddirs.h )

# Generate kdefakes.h
configure_file(kdefakes.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdefakes.h )

# Generate kdeversion.h
configure_file(util/kdeversion.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdeversion.h )

# Configure checks for kdirwatch
macro_optional_find_package(FAM)
macro_bool_to_01(FAM_FOUND HAVE_FAM)

check_include_files(sys/inotify.h SYS_INOTIFY_H_FOUND)
macro_bool_to_01(SYS_INOTIFY_H_FOUND HAVE_SYS_INOTIFY_H)
if(WIN32)
 # currently for win32 only, since it doesn't support watching files that don't exist yet
 option(USE_QFILESYSTEMWATCHER "Use QFileSystemWatcher instead polling for KDirWatch" ON)
 macro_bool_to_01(USE_QFILESYSTEMWATCHER HAVE_QFILESYSTEMWATCHER)
endif(WIN32)

# Generate io/config-kdirwatch.h
include(io/ConfigureChecks.cmake)
configure_file(io/config-kdirwatch.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/io/config-kdirwatch.h )

# Configure checks for network/ but also for netsupp.*
include(network/ConfigureChecks.cmake)
configure_file(network/config-network.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/network/config-network.h )
include_directories( ${CMAKE_CURRENT_BINARY_DIR}/network )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/sonnet/ )

# This file handles all the logic for compiling KAuth's backends
include(auth/ConfigureChecks.cmake)
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/auth/ )

# Configure a small file to tell BackendsManager what to use
configure_file(auth/BackendsConfig.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/BackendsConfig.h)

include_directories( ${KDE4_KDECORE_INCLUDES} )
include_directories( ${ZLIB_INCLUDE_DIR} )
include_directories( ${QT_INCLUDES} )

if (KDE_PLATFORM_FEATURE_DISABLE_DEPRECATED)
   set(KDECORE_NO_KDE3SUPPORT TRUE)
endif(KDE_PLATFORM_FEATURE_DISABLE_DEPRECATED)

# kdecore_OPTIONAL_SRCS is used to collect source files
# which are not always compiled into kdecore
# if it's used, *always* append to it
set(kdecore_OPTIONAL_SRCS)
# same for optional libs
set(kdecore_OPTIONAL_LIBS)

if(WIN32)
   set(kdecore_OPTIONAL_LIBS ${kdecore_OPTIONAL_LIBS} ${KDEWIN_LIBRARIES})
endif(WIN32)

# Needed for the kdatetime test in release mode
if(KDE4_BUILD_TESTS)
   add_definitions(-DCOMPILING_TESTS)
endif(KDE4_BUILD_TESTS)

# compile bzip2 support if available
if(BZIP2_FOUND)
   include_directories(${BZIP2_INCLUDE_DIR})
   set(kdecore_OPTIONAL_SRCS ${kdecore_OPTIONAL_SRCS} compression/kbzip2filter.cpp )
   set(kdecore_OPTIONAL_LIBS ${kdecore_OPTIONAL_LIBS} ${BZIP2_LIBRARIES})
endif(BZIP2_FOUND)

# compile lzma support if available
if(LIBLZMA_FOUND)
   include_directories(${LIBLZMA_INCLUDE_DIRS})
   set(kdecore_OPTIONAL_SRCS ${kdecore_OPTIONAL_SRCS} compression/kxzfilter.cpp )
   set(kdecore_OPTIONAL_LIBS ${kdecore_OPTIONAL_LIBS} ${LIBLZMA_LIBRARIES})
endif(LIBLZMA_FOUND)

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
   )

   set(kdecore_OPTIONAL_LIBS ${kdecore_OPTIONAL_LIBS} ${CARBON_LIBRARY})
endif (APPLE)

if (WIN32)
   LIST(APPEND kdecore_OPTIONAL_SRCS
      kernel/kkernel_win.cpp
      kernel/ktoolinvocation_win.cpp
      kernel/kstandarddirs_win.cpp
      network/klocalsocket_win.cpp
      io/klockfile_win.cpp
      util/kde_file_win.cpp
      util/kmacroexpander_win.cpp
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
      kernel/kstandarddirs_unix.cpp
      network/klocalsocket_unix.cpp
      network/netsupp.cpp
      io/klockfile_unix.cpp
      util/kshell_unix.cpp
      util/kuser_unix.cpp
      util/kmacroexpander_unix.cpp
      fakes.c
   )
   if (NOT KDECORE_NO_KDE3SUPPORT)
      set(kdecore_OPTIONAL_SRCS ${kdecore_OPTIONAL_SRCS}
         network/k3httpproxysocketdevice.cpp
         network/k3socks.cpp
         network/k3sockssocketdevice.cpp
      )
   endif (NOT KDECORE_NO_KDE3SUPPORT)
endif (UNIX)

add_subdirectory( kconfig_compiler )
add_subdirectory( tests )
add_subdirectory( sonnet/tests )
add_subdirectory( network/kssld )

########### next target ###############

if (UNIX)

   kde4_add_library(kdefakes SHARED fakes.c)
   set_target_properties(kdefakes PROPERTIES VERSION ${KDE_NON_GENERIC_LIB_VERSION} SOVERSION ${KDE_NON_GENERIC_LIB_SOVERSION})
   if (NOT HAVE_TRUNC)
       target_link_libraries(kdefakes  m )
   endif (NOT HAVE_TRUNC)
   install(TARGETS kdefakes EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

endif (UNIX)



########### next target ###############

set(kdecore_LIB_SRCS
   compression/kgzipfilter.cpp
   compression/kfilterbase.cpp
   compression/kfilterdev.cpp
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
   date/kcalendarsystemgregorian.cpp
   date/kcalendarsystemgregorianproleptic.cpp
   date/kcalendarsystemhijri.cpp
   date/kcalendarsystemhebrew.cpp
   date/kcalendarsystemindiannational.cpp
   date/kcalendarsystemjalali.cpp
   date/kcalendarsystemjapanese.cpp
   date/kcalendarsystemjulian.cpp
   date/kcalendarsystemminguo.cpp
   date/kcalendarsystemthai.cpp
   date/kdatetime.cpp
   date/ktimezone.cpp
   date/ksystemtimezone.cpp
   date/ktzfiletimezone.cpp
   io/kar.cpp
   io/karchive.cpp
   io/kautosavefile.cpp
   io/kdebug.cpp
   io/kdebugdbusiface.cpp
   io/kdirwatch.cpp
   io/klimitediodevice.cpp
   io/kmessage.cpp
   io/kmountpoint.cpp
   io/kprocess.cpp
   io/ksavefile.cpp
   io/ktar.cpp
   io/ktempdir.cpp
   io/ktemporaryfile.cpp
   io/kurl.cpp
   io/kzip.cpp
   jobs/kcompositejob.cpp
   jobs/kjob.cpp
   jobs/kjobuidelegate.cpp
   jobs/kjobtrackerinterface.cpp
   kernel/kauthorized.cpp
   kernel/kaboutdata.cpp
   kernel/kcmdlineargs.cpp
   kernel/kglobal.cpp
   kernel/kcomponentdata.cpp
   kernel/kstandarddirs.cpp
   kernel/ktoolinvocation.cpp
   auth/kauthaction.cpp
   auth/kauthactionreply.cpp
   auth/kauthactionwatcher.cpp
   auth/AuthBackend.cpp
   auth/BackendsManager.cpp
   auth/HelperProxy.cpp
   auth/kauthhelpersupport.cpp
   auth/backends/fake/FakeBackend.cpp
   auth/backends/fakehelper/FakeHelperProxy.cpp
   services/kfoldermimetype.cpp
   services/kmimetypefactory.cpp
   services/kmimemagicrule.cpp
   services/kmimetypetrader.cpp
   services/kmimetype.cpp
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
   sonnet/loader.cpp
   sonnet/client.cpp
   sonnet/spellerplugin.cpp
   sonnet/speller.cpp
   sonnet/filter.cpp
   sonnet/settings.cpp
   sonnet/backgroundchecker.cpp
   sonnet/backgroundengine.cpp
   sonnet/globals.cpp
)

if (NOT KDECORE_NO_KDE3SUPPORT)
   set(kdecore_LIB_SRCS ${kdecore_LIB_SRCS}
      network/k3socketdevice.cpp # must be before the rest of network/, for enable-final
      network/k3bufferedsocket.cpp
      network/k3clientsocketbase.cpp
      network/k3datagramsocket.cpp
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
   localization/kcharsets.cpp
   localization/kencodingdetector.cpp
   localization/guess_ja.cpp
   localization/kencodingprober.cpp
   localization/probers/CharDistribution.cpp
   localization/probers/ChineseGroupProber.cpp
   localization/probers/JapaneseGroupProber.cpp
   localization/probers/JpCntx.cpp
   localization/probers/LangBulgarianModel.cpp
   localization/probers/LangCyrillicModel.cpp
   localization/probers/LangGreekModel.cpp
   localization/probers/LangHebrewModel.cpp
   localization/probers/LangHungarianModel.cpp
   localization/probers/LangThaiModel.cpp
   localization/probers/UnicodeGroupProber.cpp
   localization/probers/nsBig5Prober.cpp
   localization/probers/nsCharSetProber.cpp
   localization/probers/nsEUCJPProber.cpp
   localization/probers/nsEUCKRProber.cpp
   localization/probers/nsEUCTWProber.cpp
   localization/probers/nsEscCharsetProber.cpp
   localization/probers/nsEscSM.cpp
   localization/probers/nsGB2312Prober.cpp
   localization/probers/nsHebrewProber.cpp
   localization/probers/nsLatin1Prober.cpp
   localization/probers/nsMBCSGroupProber.cpp
   localization/probers/nsMBCSSM.cpp
   localization/probers/nsSBCSGroupProber.cpp
   localization/probers/nsSBCharSetProber.cpp
   localization/probers/nsSJISProber.cpp
   localization/probers/nsUniversalDetector.cpp
   localization/klocale.cpp
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
   text/kcodecs.cpp
   text/kstringhandler.cpp
   util/kallocator.cpp
   util/kautostart.cpp
   util/kdedmodule.cpp
   util/kdeversion.cpp
   util/klauncher_iface.cpp
   util/klibloader.cpp
   util/klibrary.cpp
   util/kmacroexpander.cpp
   util/kpluginfactory.cpp
   util/kpluginloader.cpp
   util/kshell.cpp
   util/krandom.cpp
   util/krandomsequence.cpp
   util/qtest_kde.cpp
   ${kdecore_OPTIONAL_SRCS}
)

if(NOT WIN32)
    set(kdecore_LIB_SRCS ${kdecore_LIB_SRCS} util/kshareddatacache.cpp)
else(NOT WIN32)
    set(kdecore_LIB_SRCS ${kdecore_LIB_SRCS} util/kshareddatacache_win.cpp)
    set(kdecore_LIB_SRCS ${kdecore_LIB_SRCS} io/kdirwatch_win.cpp)
endif(NOT WIN32)

if (NOT Q_WS_X11 AND NOT Q_WS_QWS)
   add_definitions(-DNO_DISPLAY)
endif (NOT Q_WS_X11 AND NOT Q_WS_QWS)

kde4_add_library(kdecore SHARED ${kdecore_LIB_SRCS})

target_link_libraries(kdecore ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTDBUS_LIBRARY} ${QT_QTXML_LIBRARY} ${ZLIB_LIBRARY} ${kdecore_OPTIONAL_LIBS})

if(WINCE)
  target_link_libraries(kdecore ${WCECOMPAT_LIBRARIES} Ceshell.lib)
endif(WINCE)

set(SYS_INOTIFY_H_FOUND 0)
if(SYS_INOTIFY_H_FOUND)
   macro_log_feature(FAM_FOUND "FAM" "File Alteration Monitor" "http://oss.sgi.com/projects/fam" FALSE "" "You have file alteration notification support built into your kernel, however you might consider installing FAM as it also supports NFS.")
else(SYS_INOTIFY_H_FOUND)
   macro_log_feature(FAM_FOUND "FAM" "File Alteration Monitor" "http://oss.sgi.com/projects/fam" FALSE "" "Provides file alteration notification facilities using a separate service.")
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

# KAuth policy generator executable

# Compile only if fake backend has not been selected

if (NOT "${KDE4_AUTH_BACKEND_NAME}" STREQUAL "FAKE")
    # KAUTH_POLICY_GEN_SRCS has been generated from auth/ConfigureChecks.cmake
    kde4_add_executable(kauth-policy-gen NOGUI ${KAUTH_POLICY_GEN_SRCS})

    # KAUTH_POLICY_GEN_LIBRARIES has been generated from auth/ConfigureChecks.cmake
    target_link_libraries( kauth-policy-gen ${KAUTH_POLICY_GEN_LIBRARIES} )

    install( TARGETS kauth-policy-gen EXPORT kdelibsToolsTargets DESTINATION ${LIBEXEC_INSTALL_DIR})
endif (NOT "${KDE4_AUTH_BACKEND_NAME}" STREQUAL "FAKE")

########### next target ###############

# KAuth backend plugin

if (NOT "${KDE4_AUTH_BACKEND_NAME}" STREQUAL "FAKE")
    set(KAUTH_BACKEND_SRCS ${KAUTH_BACKEND_SRCS} auth/AuthBackend.cpp)
    kde4_add_plugin(kauth_backend_plugin ${KAUTH_BACKEND_SRCS})
    target_link_libraries(kauth_backend_plugin ${KAUTH_BACKEND_LIBS})
    install(TARGETS kauth_backend_plugin
            LIBRARY DESTINATION ${KAUTH_BACKEND_PLUGIN_DIR}
            ARCHIVE DESTINATION ${KAUTH_BACKEND_PLUGIN_DIR}
            RUNTIME DESTINATION ${KAUTH_BACKEND_PLUGIN_DIR}
    )
endif (NOT "${KDE4_AUTH_BACKEND_NAME}" STREQUAL "FAKE")

########### next target ###############

# KAuth helper plugin

if (NOT "${KDE4_AUTH_HELPER_BACKEND_NAME}" STREQUAL "FAKE")
    kde4_add_plugin(kauth_helper_plugin ${KAUTH_HELPER_BACKEND_SRCS} auth/HelperProxy.cpp)
    target_link_libraries(kauth_helper_plugin ${KAUTH_HELPER_BACKEND_LIBS})
    install(TARGETS kauth_helper_plugin
            LIBRARY DESTINATION ${KAUTH_HELPER_PLUGIN_DIR}
            ARCHIVE DESTINATION ${KAUTH_HELPER_PLUGIN_DIR}
            RUNTIME DESTINATION ${KAUTH_HELPER_PLUGIN_DIR}
    )
endif (NOT "${KDE4_AUTH_HELPER_BACKEND_NAME}" STREQUAL "FAKE")

########### next target ###############

configure_file(all_languages.desktop ${CMAKE_CURRENT_BINARY_DIR}/all_languages @ONLY)

kde4_add_executable(kde4-config NOGUI kde-config.cpp )

target_link_libraries(kde4-config ${KDE4_KDECORE_LIBS})

install(TARGETS kde4-config ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### next target ###############

if (WIN32 AND NOT WINCE)
    kde4_add_executable(kcmdwrapper NOGUI io/kcmdwrapper.cpp )

    target_link_libraries(kcmdwrapper kernel32 shell32 )

    install(TARGETS kcmdwrapper DESTINATION ${LIBEXEC_INSTALL_DIR} )
endif (WIN32 AND NOT WINCE)

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


if(KDE4_ENABLE_FINAL)
   macro_add_file_dependencies(${CMAKE_CURRENT_BINARY_DIR}/ktranscript_final_cpp.cpp ${CMAKE_CURRENT_BINARY_DIR}/ktranscript.lut.h)
else(KDE4_ENABLE_FINAL)
   macro_add_file_dependencies(${CMAKE_CURRENT_SOURCE_DIR}/localization/ktranscript.cpp ${CMAKE_CURRENT_BINARY_DIR}/ktranscript.lut.h)
endif(KDE4_ENABLE_FINAL)

install( TARGETS ktranscript DESTINATION ${PLUGIN_INSTALL_DIR} )

endif(NOT WINCE)

########### install files ###############
install( FILES ${CMAKE_CURRENT_BINARY_DIR}/all_languages DESTINATION ${LOCALE_INSTALL_DIR}  )
install( FILES kdebug.areas kdebugrc DESTINATION ${CONFIG_INSTALL_DIR} )
install( FILES
   kdecore_export.h
   compression/kfilterbase.h
   compression/kfilterdev.h
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
   date/ksystemtimezone.h
   date/ktimezone.h
   date/ktzfiletimezone.h
   io/kar.h
   io/karchive.h
   io/kautosavefile.h
   io/kdebug.h
   io/kdirwatch.h
   io/kmessage.h
   io/kprocess.h
   io/klockfile.h
   io/kmountpoint.h
   io/ksavefile.h
   io/ktar.h
   io/ktempdir.h
   io/ktemporaryfile.h
   io/kurl.h
   io/kzip.h
   jobs/kcompositejob.h
   jobs/kjob.h
   jobs/kjobuidelegate.h
   jobs/kjobtrackerinterface.h
   kernel/kaboutdata.h
   kernel/kauthorized.h
   kernel/kcmdlineargs.h
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
      network/k3datagramsocket.h
      network/k3httpproxysocketdevice.h
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
   auth/kauthaction.h
   auth/kauthactionreply.h
   auth/kauthactionwatcher.h
   auth/kauthhelpersupport.h
   auth/kauth.h
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
   localization/kcharsets.h
   localization/kcurrencycode.h
   localization/kencodingdetector.h
   localization/kencodingprober.h
   localization/klocale.h
   localization/klocalizedstring.h
   sycoca/kprotocolinfo.h
   sycoca/ksycoca.h
   sycoca/ksycocaentry.h
   sycoca/ksycocatype.h
   text/kascii.h
   text/kcodecs.h
   text/kstringhandler.h
   util/kallocator.h
   util/kautostart.h
   util/kde_file.h
   util/kdedmodule.h
   util/kgenericfactory.h
   util/kgenericfactory.tcc
   util/klauncher_iface.h
   util/klibloader.h
   util/klibrary.h
   util/kmacroexpander.h
   util/kexportplugin.h
   util/kpluginfactory.h
   util/kpluginloader.h
   util/kshell.h
   util/krandom.h
   util/krandomsequence.h
   util/kshareddatacache.h
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

install( FILES sonnet/backgroundchecker.h sonnet/speller.h sonnet/globals.h
         DESTINATION ${INCLUDE_INSTALL_DIR}/sonnet COMPONENT Devel)

install( FILES
   config/kconfigbackend.desktop
   services/kplugininfo.desktop
   sonnet/sonnetspeller.desktop
   DESTINATION  ${SERVICETYPES_INSTALL_DIR}
)


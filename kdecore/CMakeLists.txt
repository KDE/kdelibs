project(kdecore)
# Generate kdefakes.h
configure_file(kdefakes.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdefakes.h )

# Configure checks for network/ but also for netsupp.*
include(network/ConfigureChecks.cmake)
configure_file(network/config-network.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/network/config-network.h )
include_directories( ${CMAKE_CURRENT_BINARY_DIR}/network )

include_directories( ${KDE4_KDECORE_INCLUDES} )
include_directories( ${ZLIB_INCLUDE_DIR} )
include_directories( ${QT_INCLUDES} )

# kdecore_OPTIONAL_SRCS is used to collect source files
# which are not always compiled into kdecore
# if it's used, *always* append to it
set(kdecore_OPTIONAL_SRCS)
# same for optional libs
set(kdecore_OPTIONAL_LIBS)

if(WIN32)
   set(kdecore_OPTIONAL_LIBS ${kdecore_OPTIONAL_LIBS} ${KDEWIN32_LIBRARIES})
endif(WIN32)

# compile bzip2 support if available
if(BZIP2_FOUND)
   include_directories(${BZIP2_INCLUDE_DIR})
   set(kdecore_OPTIONAL_SRCS ${kdecore_OPTIONAL_SRCS} compression/kbzip2filter.cpp )
   set(kdecore_OPTIONAL_LIBS ${kdecore_OPTIONAL_LIBS} ${BZIP2_LIBRARIES})
endif(BZIP2_FOUND)

# compile Gettext support if available
if(GETTEXT_FOUND)
  include_directories(${GETTEXT_INCLUDE_DIR})
  set(kdecore_OPTIONAL_LIBS ${kdecore_OPTIONAL_LIBS} ${GETTEXT_LIBRARIES})
endif(GETTEXT_FOUND)


if (HAVE_RESOLV_LIBRARY)
  set(kdecore_OPTIONAL_LIBS ${kdecore_OPTIONAL_LIBS} resolv)
endif (HAVE_RESOLV_LIBRARY)

if(HAVE_IDNA_H)
   set(kdecore_OPTIONAL_LIBS ${kdecore_OPTIONAL_LIBS} idn)
endif(HAVE_IDNA_H)

if(X11_XTest_FOUND)
   set(kdecore_OPTIONAL_LIBS ${kdecore_OPTIONAL_LIBS} ${X11_XTest_LIB})
endif(X11_XTest_FOUND)

if (APPLE)
   set(kdecore_OPTIONAL_LIBS ${kdecore_OPTIONAL_LIBS} ${CARBON_LIBRARY})
endif (APPLE)

if (WIN32)
   set(kdecore_OPTIONAL_SRCS ${kdecore_OPTIONAL_SRCS}
      kernel/kkernel_win.cpp
      kernel/ktoolinvocation_win.cpp
      network/klocalsocket_win.cpp
      )
endif (WIN32)


if (UNIX)
   set(kdecore_OPTIONAL_SRCS ${kdecore_OPTIONAL_SRCS}
      kernel/ktoolinvocation_x11.cpp 
      network/khttpproxysocketdevice.cpp
      network/ksocks.cpp
      network/ksockssocketdevice.cpp
      network/klocalsocket_unix.cpp
      util/kpty.cpp
      fakes.c )
endif (UNIX)


add_subdirectory( kconfig_compiler )
add_subdirectory( tests )
add_subdirectory( network/tests )

########### next target ###############

if (UNIX)

   kde4_add_library(kdefakes SHARED fakes.c)
   set_target_properties(kdefakes PROPERTIES VERSION 5.0.0 SOVERSION 5 )
   install(TARGETS kdefakes DESTINATION ${LIB_INSTALL_DIR} )

   #no ttyname on windows
   kde4_add_executable(kgrantpty NOGUI kgrantpty.c)
   target_link_libraries(kgrantpty  kdefakes )
   install(TARGETS kgrantpty DESTINATION ${LIBEXEC_INSTALL_DIR} )
   
endif (UNIX)



########### next target ###############

set(kdecore_LIB_SRCS
   compression/kgzipfilter.cpp
   compression/kfilterbase.cpp
   compression/kfilterdev.cpp
   config/kconfig.cpp
   config/kconfigbase.cpp
   config/kconfiggroup.cpp
   config/kconfigskeleton.cpp
   config/kconfigbackend.cpp
   config/kconfigini.cpp
   config/kdesktopfile.cpp
   config/ksharedconfig.cpp
   date/kcalendarsystem.cpp
   date/kcalendarsystemgregorian.cpp
   date/kcalendarsystemhijri.cpp
   date/kcalendarsystemhebrew.cpp
   date/kcalendarsystemjalali.cpp
   date/kdatetime.cpp
   date/ktimezones.cpp
   date/ksystemtimezone.cpp
   date/ktzfiletimezone.cpp
   io/kautosavefile.cpp
   io/kdebug.cpp
   io/kdebugdbusiface.cpp
   io/klockfile.cpp
   io/kmessage.cpp
   io/kmountpoint.cpp
   io/k3process.cpp
   io/k3processcontroller.cpp
   io/k3procio.cpp
   io/ksavefile.cpp
   io/ktempdir.cpp
   io/ktemporaryfile.cpp
   io/kurl.cpp
   jobs/kcompositejob.cpp
   jobs/kjob.cpp
   jobs/kjobuidelegate.cpp
   jobs/kjobtrackerinterface.cpp
   kernel/kauthorized.cpp
   kernel/kautostart.cpp
   kernel/kaboutdata.cpp
   kernel/kcmdlineargs.cpp
   kernel/kglobal.cpp
   kernel/kcomponentdata.cpp
   kernel/kstandarddirs.cpp
   kernel/ktoolinvocation.cpp
   services/kdedesktopmimetype.cpp
   services/kmimetypefactory.cpp
   services/kmimemagicrule.cpp
   services/kmimetypetrader.cpp
   services/kmimetype.cpp
   services/kservice.cpp
   services/kservicefactory.cpp
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
   network/ksocketdevice.cpp # must be before the rest of network/, for enable-final
   network/kbufferedsocket.cpp
   network/kclientsocketbase.cpp
   network/kdatagramsocket.cpp
   network/kidna.cpp
   network/kresolver.cpp
   network/kresolvermanager.cpp
   network/kresolverworkerbase.cpp
   network/kresolverstandardworkers.cpp
   network/kreverseresolver.cpp
   network/kserversocket.cpp
   network/ksocketaddress.cpp
   network/ksocketbase.cpp
   network/kstreamsocket.cpp
   network/ksocketbuffer.cpp
   network/klocalsocket.cpp
   network/netsupp.cpp
   localization/kcatalog.cpp
   localization/kcharsets.cpp
   localization/klocale.cpp
   localization/klocalizedstring.cpp
   sycoca/ksycoca.cpp
   sycoca/ksycocadict.cpp
   sycoca/ksycocaentry.cpp
   sycoca/ksycocafactory.cpp
   sycoca/kprotocolinfo.cpp
   sycoca/kprotocolinfofactory.cpp
   text/kascii.cpp
   text/kcodecs.cpp
   text/kstringhandler.cpp
   util/kallocator.cpp
   util/kdedmodule.cpp
   util/kdeversion.cpp
   util/klauncher_iface.cpp
   util/klibloader.cpp
   util/kmacroexpander.cpp
   util/kshell.cpp
   util/krandom.cpp
   util/krandomsequence.cpp
   util/kstaticdeleter.cpp
   util/kuser.cpp
   ${kdecore_OPTIONAL_SRCS}
)

kde4_automoc(${kdecore_LIB_SRCS})

if (NOT Q_WS_X11 AND NOT Q_WS_QWS)
add_definitions(-DNO_DISPLAY)
endif (NOT Q_WS_X11 AND NOT Q_WS_QWS)

kde4_add_library(kdecore SHARED ${kdecore_LIB_SRCS})

target_link_libraries(kdecore  ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTDBUS_LIBRARY} ${UTIL_LIBRARY} ${ZLIB_LIBRARY} ${kdecore_OPTIONAL_LIBS})

set_target_properties(kdecore PROPERTIES VERSION 5.0.0 SOVERSION 5 )
install(TARGETS kdecore DESTINATION ${LIB_INSTALL_DIR} )


########### next target ###############

# set the variables used in kde-config.cpp.in

set(prefix              ${CMAKE_INSTALL_PREFIX})
set(datadir             ${DATA_INSTALL_DIR})
set(exec_prefix         ${BIN_INSTALL_DIR})
set(libdir              ${LIB_INSTALL_DIR})
set(includedir          ${INCLUDE_INSTALL_DIR})
set(kde_appsdir         ${APPLNK_INSTALL_DIR} )
set(kde_bindir          ${BIN_INSTALL_DIR} )
set(kde_confdir         ${CONFIG_INSTALL_DIR})
set(kde_datadir         ${DATA_INSTALL_DIR})
set(kde_htmldir         ${HTML_INSTALL_DIR})
set(kde_icondir         ${ICON_INSTALL_DIR})
set(kde_kcfgdir         ${KCFG_INSTALL_DIR})
set(kde_locale          ${LOCALE_INSTALL_DIR})
set(kde_moduledir       ${PLUGIN_INSTALL_DIR})
set(kde_mimedir         ${MIME_INSTALL_DIR})
set(kde_servicesdir     ${SERVICES_INSTALL_DIR})
set(kde_servicetypesdir ${SERVICETYPES_INSTALL_DIR})
set(kde_sounddir        ${SOUND_INSTALL_DIR})
set(kde_templatesdir    ${TEMPLATES_INSTALL_DIR})
set(kde_wallpaperdir    ${WALLPAPER_INSTALL_DIR})
set(xdg_appsdir         ${XDG_APPS_DIR})
set(xdg_directorydir    ${XDG_DIRECTORY_DIR})


configure_file(kde-config.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/kde-config.cpp @ONLY)
configure_file(all_languages.desktop ${CMAKE_CURRENT_BINARY_DIR}/all_languages @ONLY)  

kde4_add_executable(kde4-config NOGUI ${CMAKE_CURRENT_BINARY_DIR}/kde-config.cpp )

target_link_libraries(kde4-config ${KDE4_KDECORE_LIBS} )

install(TARGETS kde4-config DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############
install( FILES ${CMAKE_CURRENT_BINARY_DIR}/all_languages DESTINATION ${LOCALE_INSTALL_DIR}  )
install( FILES kdebug.areas kdebugrc localization/language.codes DESTINATION ${CONFIG_INSTALL_DIR} )
install( FILES
   kdecore_export.h
   compression/kfilterbase.h
   compression/kfilterdev.h
   config/kconfig.h
   config/kconfigbase.h
   config/kconfiggroup.h
   config/kconfigbackend.h
   config/kconfigini.h
   config/kconfigskeleton.h
   config/kconfigdata.h
   config/kdesktopfile.h
   config/ksharedconfig.h
   config/kconfigflags.h
   date/kcalendarsystem.h
   date/kdatetime.h
   date/ksystemtimezone.h
   date/ktimezones.h
   date/ktzfiletimezone.h
   io/kautosavefile.h
   io/kdebug.h
   io/kmessage.h
   io/k3process.h
   io/k3processcontroller.h
   io/k3procio.h
   io/klockfile.h
   io/kmountpoint.h
   io/ksavefile.h
   io/ktempdir.h
   io/ktemporaryfile.h
   io/kurl.h
   jobs/kcompositejob.h
   jobs/kjob.h
   jobs/kjobuidelegate.h
   jobs/kjobtrackerinterface.h
   kernel/kaboutdata.h
   kernel/kauthorized.h
   kernel/kautostart.h
   kernel/kcmdlineargs.h
   kernel/kglobal.h
   kernel/kcomponentdata.h
   kernel/kstandarddirs.h
   kernel/ktoolinvocation.h
   network/kbufferedsocket.h
   network/kclientsocketbase.h
   network/kdatagramsocket.h
   network/khttpproxysocketdevice.h
   network/kidna.h
   network/kiobuffer.h
   network/kresolver.h
   network/kreverseresolver.h
   network/kserversocket.h
   network/ksocketaddress.h
   network/ksocketbase.h
   network/ksocketdevice.h
   network/ksocks.h
   network/ksockssocketdevice.h
   network/kstreamsocket.h
   services/kmimetype.h
   services/kmimetypetrader.h
   services/kservice.h
   #services/kservicefactory.h: do not install, internal API
   services/kservicetype.h
   #services/kservicetypefactory.h: do not install, internal API
   services/kservicetypeprofile.h
   services/kservicetypetrader.h
   services/kserviceoffer.h
   services/kplugininfo.h
   localization/kcatalog.h
   localization/kcharsets.h
   localization/klocale.h
   localization/klocalizedstring.h
   sycoca/kprotocolinfo.h
   sycoca/ksycoca.h
   sycoca/ksycocaentry.h
   sycoca/ksycocatype.h
   text/kascii.h
   text/kcodecs.h
   text/kstringhandler.h
   util/conversion_check.h
   util/kallocator.h
   util/kde_file.h
   util/kdedmodule.h
   util/kdeversion.h
   util/kgenericfactory.h
   util/kgenericfactory.tcc
   util/klauncher_iface.h
   util/klibloader.h
   util/kmacroexpander.h
   util/kpty.h
   util/kshell.h
   util/krandom.h
   util/krandomsequence.h
   util/ksharedptr.h
   util/ksortablelist.h
   util/kstaticdeleter.h
   util/ktypelist.h
   util/kuser.h
   util/qtest_kde.h
   ${CMAKE_CURRENT_BINARY_DIR}/kdefakes.h
   DESTINATION ${INCLUDE_INSTALL_DIR}
)

install( FILES
   services/kplugininfo.desktop
   DESTINATION  ${SERVICETYPES_INSTALL_DIR}
)

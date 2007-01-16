project(kdecore)
KDE4_NO_ENABLE_FINAL(kdecore)
# Generate kdefakes.h
configure_file(kdefakes.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdefakes.h )

# Configure checks for network/ but also for netsupp.*
include(network/ConfigureChecks.cmake)
configure_file(config-network.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-network.h )

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
      kernel/ktoolinvocation_win.cpp 
      )
endif (WIN32)


if (UNIX)
   set(kdecore_OPTIONAL_SRCS ${kdecore_OPTIONAL_SRCS}
      kernel/ktoolinvocation_x11.cpp 
      kpty.cpp
      network/khttpproxysocketdevice.cpp
      network/ksocks.cpp
      network/ksockssocketdevice.cpp
      fakes.c )
endif (UNIX)


add_subdirectory( kconfig_compiler )
add_subdirectory( tests )
add_subdirectory( network )
add_subdirectory( services )

########### next target ###############

if (UNIX)

   kde4_add_library(kdefakes SHARED fakes.c)
   set_target_properties(kdefakes PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
   install(TARGETS kdefakes DESTINATION ${LIB_INSTALL_DIR} )

   #no ttyname on windows
   kde4_add_executable(kgrantpty NOGUI kgrantpty.c)
   target_link_libraries(kgrantpty  kdefakes )
   install(TARGETS kgrantpty DESTINATION ${LIBEXEC_INSTALL_DIR} )
   
endif (UNIX)



########### next target ###############

set(kdecore_LIB_SRCS
   kallocator.cpp
   kckey.cpp
   kcodecs.cpp
   kcompositejob.cpp
   kdebug.cpp
   kdebugdbusiface.cpp
   kdedmodule.cpp
   kdeversion.cpp
   kjob.cpp
   kjobuidelegate.cpp
   klauncher_iface.cpp
   klibloader.cpp
   kmacroexpander.cpp
   kmessage.cpp
   krandom.cpp
   krandomsequence.cpp
   kshell.cpp
   kstaticdeleter.cpp
   kuser.cpp
   compression/kgzipfilter.cpp
   compression/kfilterbase.cpp
   compression/kfilterdev.cpp
   config/kconfig.cpp
   config/kconfigbase.cpp
   config/kconfigskeleton.cpp
   config/kconfigbackend.cpp
   config/kdesktopfile.cpp
   config/ksimpleconfig.cpp
   date/kcalendarsystemjalali.cpp
   date/krfcdate.cpp
   io/kautosavefile.cpp
   io/klockfile.cpp
   io/kmountpoint.cpp
   io/kprocess.cpp
   io/kprocctrl.cpp
   io/kprocio.cpp
   io/ksavefile.cpp
   io/ktempdir.cpp
   io/ktemporaryfile.cpp
   io/kurl.cpp
   kernel/kauthorized.cpp
   kernel/kautostart.cpp
   kernel/kaboutdata.cpp
   kernel/kcmdlineargs.cpp
   kernel/kglobal.cpp
   kernel/kinstance.cpp
   kernel/kstandarddirs.cpp
   kernel/ktoolinvocation.cpp
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
   network/kresolver.cpp
   network/kresolvermanager.cpp
   network/kresolverworkerbase.cpp
   network/ksocketaddress.cpp
   network/kresolverstandardworkers.cpp
   network/kreverseresolver.cpp
   network/ksocketdevice.cpp
   network/ksocketbase.cpp
   network/kclientsocketbase.cpp
   network/kstreamsocket.cpp
   network/kserversocket.cpp
   network/kdatagramsocket.cpp
   network/kbufferedsocket.cpp
   network/ksocketbuffer.cpp
   network/netsupp.cpp
   strings/kascii.cpp
   strings/kcharsets.cpp
   strings/klocale.cpp
   strings/klocalizedstring.cpp
   strings/kstringhandler.cpp
   sycoca/ksycoca.cpp
   sycoca/ksycocadict.cpp
   sycoca/ksycocafactory.cpp
   strings/kcatalog.cpp
   date/kcalendarsystem.cpp
   date/kcalendarsystemgregorian.cpp
   date/kcalendarsystemhijri.cpp
   date/kcalendarsystemhebrew.cpp
   date/kcalendarsystemfactory.cpp
   strings/kidna.cpp
   sycoca/kprotocolinfo.cpp
   sycoca/kprotocolinfofactory.cpp
   date/ktimezones.cpp
   date/ksystemtimezone.cpp
   date/ktzfiletimezone.cpp
   date/kdatetime.cpp
   ${kdecore_OPTIONAL_SRCS}
)

kde4_automoc(${kdecore_LIB_SRCS})

if (NOT Q_WS_X11 AND NOT Q_WS_QWS)
add_definitions(-DNO_DISPLAY)
endif (NOT Q_WS_X11 AND NOT Q_WS_QWS)

kde4_add_library(kdecore SHARED ${kdecore_LIB_SRCS})

target_link_libraries(kdecore  ${QT_QTCORE_LIBRARY} ${QT_QTSVG_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTDBUS_LIBRARY} ${UTIL_LIBRARY} ${ZLIB_LIBRARY} ${kdecore_OPTIONAL_LIBS})

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
install( FILES kdebug.areas kdebugrc language.codes DESTINATION ${CONFIG_INSTALL_DIR})
install( FILES
   kcompositejob.h
   kjob.h
   kjobuidelegate.h
   klauncher_iface.h
   kdeversion.h
   kpty.h
   kdebug.h
   ksharedptr.h
   kallocator.h
   klibloader.h
   krandomsequence.h
   kstaticdeleter.h
   krandom.h
   kcodecs.h
   kgenericfactory.h
   kgenericfactory.tcc
   ktypelist.h
   ksortablelist.h
   kuser.h
   kshell.h
   kde_file.h
   kmacroexpander.h
   qtest_kde.h
   conversion_check.h
   kdedmodule.h
   kmessage.h
   compression/kfilterbase.h
   compression/kfilterdev.h
   config/kconfig.h
   config/kconfigbase.h
   config/kconfigbackend.h
   config/kconfigskeleton.h
   config/kconfigdata.h
   config/kdesktopfile.h
   config/ksimpleconfig.h
   date/kcalendarsystem.h
   date/kcalendarsystemfactory.h
   date/kdatetime.h
   date/krfcdate.h
   date/ksystemtimezone.h
   date/ktimezones.h
   date/ktzfiletimezone.h
   io/kautosavefile.h
   io/kprocess.h
   io/kprocctrl.h
   io/kprocio.h
   io/klockfile.h
   io/kmountpoint.h
   io/ksavefile.h
   io/ktempdir.h
   io/ktemporaryfile.h
   io/kurl.h
   kernel/kaboutdata.h
   kernel/kauthorized.h
   kernel/kautostart.h
   kernel/kcmdlineargs.h
   kernel/kglobal.h
   kernel/kinstance.h
   kernel/kstandarddirs.h
   kernel/ktoolinvocation.h
   strings/kascii.h
   strings/kcatalog.h
   strings/kcharsets.h
   strings/kidna.h
   strings/klocale.h
   strings/klocalizedstring.h
   strings/kstringhandler.h
   network/ksocks.h
   sycoca/kprotocolinfo.h
   sycoca/ksycoca.h
   sycoca/ksycocaentry.h
   sycoca/ksycocatype.h
   ${CMAKE_CURRENT_BINARY_DIR}/kdefakes.h
   DESTINATION ${INCLUDE_INSTALL_DIR}
)

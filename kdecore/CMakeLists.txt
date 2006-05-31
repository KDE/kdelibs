project(kdecore)

# Generate kdefakes.h
configure_file(kdefakes.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdefakes.h )

# Configure checks for network/ but also for netsupp.*
include(network/ConfigureChecks.cmake)
configure_file(config-network.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-network.h )

include_directories( ${CMAKE_SOURCE_DIR}/libltdl )
# hack for kacceleratormanager
include_directories( ${CMAKE_SOURCE_DIR}/kdeui )

# our own subdirs
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/network )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/kservice )

include_directories( ${ZLIB_INCLUDE_DIR} )
include_directories( ${QT_INCLUDES} )
include_directories( ${QDBUS_INCLUDE_DIRS} )
#include_directories( ${KDE4_DCOP_INCLUDES} )
add_definitions( ${QDBUS_DEFINITIONS} )

# kdecore_OPTIONAL_SRCS is used to collect source files
# which are not always compiled into kdecore
# if it's used, *always* append to it
set(kdecore_OPTIONAL_SRCS)
# same for optional libs
set(kdecore_OPTIONAL_LIBS)

# no more qt3support needed (but we still need qt3support.lib!)
remove_definitions(-DQT3_SUPPORT_WARNINGS)
remove_definitions(-DQT3_SUPPORT)

# compile bzip2 support if available
if(BZIP2_FOUND)
   include_directories(${BZIP2_INCLUDE_DIR})
   set(kdecore_OPTIONAL_SRCS ${kdecore_OPTIONAL_SRCS} kbzip2filter.cpp )
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

if(HAVE_LIBDL)
   set(kdecore_OPTIONAL_LIBS ${kdecore_OPTIONAL_LIBS} ${LIBDL})
endif(HAVE_LIBDL)

if(X11_XTest_FOUND)
   set(kdecore_OPTIONAL_LIBS ${kdecore_OPTIONAL_LIBS} ${X11_XTest_LIB})
endif(X11_XTest_FOUND)

if (APPLE)
   set(kdecore_OPTIONAL_LIBS ${kdecore_OPTIONAL_LIBS} ${CARBON_LIBRARY})
endif (APPLE)

if (WIN32)
   set(kdecore_OPTIONAL_SRCS ${kdecore_OPTIONAL_SRCS}
      ${CMAKE_SOURCE_DIR}/libltdl/ltdl_win.c
      kapplication_win.cpp 
      ktoolinvocation_win.cpp 
      #netsupp_win32.cpp is obsolete
      )

    add_definitions(-DKIO_EXPORT=)
endif (WIN32)


if (UNIX)
   set(kdecore_OPTIONAL_SRCS ${kdecore_OPTIONAL_SRCS}
      ktoolinvocation_x11.cpp 
      ksocks.cpp
      netsupp.cpp
      netwm.cpp
      kpty.cpp
      kwin.cpp
      kwinmodule.cpp 
      network/khttpproxysocketdevice.cpp
      network/ksockssocketdevice.cpp
      network/knetworkinterface.cpp 
      fakes.c )
endif (UNIX)


add_subdirectory( malloc )
add_subdirectory( kconfig_compiler )
add_subdirectory( tests )
add_subdirectory( network )
add_subdirectory( kservice )

########### next target ###############

if (UNIX)

   kde4_add_library(kdefakes SHARED fakes.c)
   set_target_properties(kdefakes PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
   install_targets(${LIB_INSTALL_DIR} kdefakes )

   #no ttyname on windows
   kde4_add_executable(kgrantpty NOGUI kgrantpty.c)
   target_link_libraries(kgrantpty  kdefakes )
   install(TARGETS kgrantpty DESTINATION bin)
   
endif (UNIX)



########### next target ###############

set(kdecore_LIB_SRCS
   kapplication.cpp
   kascii.cpp
   kauthorized.cpp
   kautostart.cpp
   kdebug.cpp
   kconfigbase.cpp
   kconfig.cpp
   ksimpleconfig.cpp
   kconfigbackend.cpp
   kdesktopfile.cpp
   kstandarddirs.cpp
   kprocess.cpp
   kprocctrl.cpp
   klocale.cpp
   klocalizedstring.cpp
   krfcdate.cpp
   kiconeffect.cpp
   kicontheme.cpp
   kiconloader.cpp
   kcharsets.cpp
   kckey.cpp
   kshortcut.cpp
   kstdaccel.cpp
   kcrash.cpp
   kurl.cpp
   kjob.cpp
   kglobal.cpp
   kglobalsettings.cpp
   kallocator.cpp
   kvmallocator.cpp
   kmimesourcefactory.cpp
   kinstance.cpp
   kpalette.cpp
   kipc.cpp
   klibloader.cpp
   ktempfile.cpp
   kuniqueapplication.cpp
   kacceleratormanager.cpp
   ksavefile.cpp
   krandomsequence.cpp
   kstringhandler.cpp
   kcmdlineargs.cpp
   kaboutdata.cpp
   knotifyclient.cpp
   kprocio.cpp
   krandom.cpp
   kpixmapprovider.cpp
   ktoolinvocation.cpp
   klauncher_iface.cpp
   kcodecs.cpp
   ksycoca.cpp
   ksycocadict.cpp
   ksycocafactory.cpp
   kxmessages.cpp
   kstartupinfo.cpp
   kcatalog.cpp
   kstaticdeleter.cpp
   kclipboard.cpp
   kcheckaccelerators.cpp
   kdeversion.cpp
   kdebugdbusiface.cpp
   kcalendarsystem.cpp
   kcalendarsystemgregorian.cpp
   kcalendarsystemhijri.cpp
   kcalendarsystemhebrew.cpp
   kcalendarsystemfactory.cpp
   kmacroexpander.cpp
   kidna.cpp
   ktempdir.cpp
   kshell.cpp
   kmountpoint.cpp
   kcalendarsystemjalali.cpp
   kprotocolinfo.cpp
   kprotocolinfofactory.cpp
   kxerrorhandler.cpp
   kuser.cpp
   kconfigskeleton.cpp
   klockfile.cpp
   ktimezones.cpp
   ktzfiletimezone.cpp
   kdatetime.cpp
   kfilterbase.cpp
   kfilterdev.cpp
   kdedmodule.cpp
   kgzipfilter.cpp
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
   ${kdecore_OPTIONAL_SRCS}
   kservice/kservice.cpp
   kservice/kservicefactory.cpp
   kservice/kservicetype.cpp
   kservice/kservicetypefactory.cpp
   kservice/kservicetypeprofile.cpp
   kservice/kservicetypetrader.cpp
   kservice/ktraderparse.cpp
   kservice/ktraderparsetree.cpp
   kservice/yacc.c
   kservice/lex.c
   malloc/malloc.c
   ${CMAKE_SOURCE_DIR}/libltdl/ltdl.c
)

kde4_automoc(${kdecore_LIB_SRCS})

if (NOT Q_WS_X11 AND NOT Q_WS_QWS)
add_definitions(-DNO_DISPLAY)
endif (NOT Q_WS_X11 AND NOT Q_WS_QWS)

kde4_add_library(kdecore SHARED ${kdecore_LIB_SRCS})

target_link_libraries(kdecore  ${QT_QTCORE_LIBRARY} ${QT_QT3SUPPORT_LIBRARY} ${QT_QTSVG_LIBRARY} ${QT_QTGUI_LIBRARY} ${X11_LIBRARIES} ${UTIL_LIBRARY} ${QDBUS_LIBRARIES} ${ZLIB_LIBRARY} ${kdecore_OPTIONAL_LIBS})

set_target_properties(kdecore PROPERTIES VERSION 5.0.0 SOVERSION 5 )
install_targets(${LIB_INSTALL_DIR} kdecore )


########### next target ###############

# set the variables used in kde-config.cpp.in

set(prefix              ${CMAKE_INSTALL_PREFIX})
set(datadir             ${CMAKE_INSTALL_PREFIX}${DATA_INSTALL_DIR})
set(exec_prefix         ${CMAKE_INSTALL_PREFIX}/bin)
set(libdir              ${CMAKE_INSTALL_PREFIX}${LIB_INSTALL_DIR})
set(includedir          ${CMAKE_INSTALL_PREFIX}/include)
set(kde_appsdir         ${CMAKE_INSTALL_PREFIX}${APPLNK_INSTALL_DIR} )
set(kde_bindir          ${CMAKE_INSTALL_PREFIX}/bin )
set(kde_confdir         ${CMAKE_INSTALL_PREFIX}${CONFIG_INSTALL_DIR})
set(kde_datadir         ${CMAKE_INSTALL_PREFIX}${DATA_INSTALL_DIR})
set(kde_htmldir         ${CMAKE_INSTALL_PREFIX}${HTML_INSTALL_DIR})
set(kde_icondir         ${CMAKE_INSTALL_PREFIX}${ICON_INSTALL_DIR})
set(kde_kcfgdir         ${CMAKE_INSTALL_PREFIX}${KCFG_INSTALL_DIR})
set(kde_locale          ${CMAKE_INSTALL_PREFIX}${LOCALE_INSTALL_DIR})
set(kde_moduledir       ${CMAKE_INSTALL_PREFIX}${PLUGIN_INSTALL_DIR})
set(kde_mimedir         ${CMAKE_INSTALL_PREFIX}${MIME_INSTALL_DIR})
set(kde_servicesdir     ${CMAKE_INSTALL_PREFIX}${SERVICES_INSTALL_DIR})
set(kde_servicetypesdir ${CMAKE_INSTALL_PREFIX}${SERVICETYPES_INSTALL_DIR})
set(kde_sounddir        ${CMAKE_INSTALL_PREFIX}${SOUND_INSTALL_DIR})
set(kde_templatesdir    ${CMAKE_INSTALL_PREFIX}${TEMPLATES_INSTALL_DIR})
set(kde_wallpaperdir    ${CMAKE_INSTALL_PREFIX}${WALLPAPER_INSTALL_DIR})
set(xdg_appsdir         ${CMAKE_INSTALL_PREFIX}${XDG_APPS_DIR})
set(xdg_directorydir    ${CMAKE_INSTALL_PREFIX}${XDG_DIRECTORY_DIR})


configure_file(kde-config.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/kde-config.cpp @ONLY)
configure_file(all_languages.desktop ${CMAKE_CURRENT_BINARY_DIR}/all_languages @ONLY)  

#macro_additional_clean_files( ${CMAKE_CURRENT_BINARY_DIR}/kde-config.cpp  ${CMAKE_CURRENT_BINARY_DIR}/all_languages )

kde4_add_executable(kde-config NOGUI ${CMAKE_CURRENT_BINARY_DIR}/kde-config.cpp )

target_link_libraries(kde-config ${KDE4_KDECORE_LIBS} )

install(TARGETS kde-config DESTINATION bin)


########### install files ###############
install_files( ${LOCALE_INSTALL_DIR} FILES all_languages )
install_files( ${DATA_INSTALL_DIR}/knotify FILES eventsrc )
install_files( ${CONFIG_INSTALL_DIR} FILES kdebug.areas kdebugrc language.codes )
install_files( ${CONFIG_INSTALL_DIR}/colors FILES 40.colors Web.colors Royal.colors Rainbow.colors )
install_files( /include FILES
   kascii.h
   kconfig.h
   kconfigskeleton.h
   kconfigdata.h
   ksimpleconfig.h
   kconfigbase.h
   kdesktopfile.h
   kurl.h
   kaboutdata.h
   kjob.h
   kcmdlineargs.h
   kconfigbackend.h
   kapplication.h
   kauthorized.h
   klauncher_iface.h
   ktoolinvocation.h
   kuniqueapplication.h
   kcharsets.h
   kdeversion.h
   kpty.h
   kprocess.h
   kprocctrl.h
   klocale.h
   klocalizedstring.h
   kicontheme.h
   kiconloader.h
   kdebug.h
   kwinmodule.h
   kwin.h
   kshortcut.h
   kstdaccel.h
   kcatalog.h
   kstringhandler.h
   kstandarddirs.h
   kglobal.h
   kglobalsettings.h
   ksharedptr.h
   kallocator.h
   kvmallocator.h
   kcrash.h
   krfcdate.h
   kinstance.h
   kpalette.h
   kipc.h
   klibloader.h
   ktempfile.h
   ksavefile.h
   krandomsequence.h
   knotifyclient.h
   kiconeffect.h
   netwm.h
   kacceleratormanager.h
   netwm_def.h
   kpixmapprovider.h
   kstaticdeleter.h
   kprocio.h
   kasyncio.h
   krandom.h
   kbufferedio.h
   kmimesourcefactory.h
   kcodecs.h
   ksocks.h
   ksycoca.h
   ksycocaentry.h
   ksycocatype.h
   kxmessages.h
   kstartupinfo.h
   kgenericfactory.h
   kgenericfactory.tcc
   ktypelist.h
   ksortablelist.h
   kclipboard.h
   kcalendarsystem.h
   kcalendarsystemfactory.h
   kmacroexpander.h
   kmountpoint.h
   kuser.h
   klockfile.h
   kidna.h
   ktempdir.h
   kshell.h
   fixx11h.h
   kxerrorhandler.h
   kde_file.h
   ktimezones.h
   ktzfiletimezone.h
   kfilterbase.h
   kfilterdev.h
   kdatetime.h
   qtest_kde.h
   conversion_check.h
   kdedmodule.h
   kautostart.h
   kprotocolinfo.h
   kdefakes.h
)


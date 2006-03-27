kde4_header()

project(kdecore)

include_directories( ${CMAKE_SOURCE_DIR}/libltdl )
# hack for kacceleratormanager
include_directories( ${CMAKE_SOURCE_DIR}/kdeui )

# our own subdirs
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/network )

include_directories( ${ZLIB_INCLUDE_DIR} )
include_directories( ${KDE4_DCOP_INCLUDES} )


# kdecore_OPTIONAL_SRCS is used to collect source files
# which are not always compiled into kdecore
# if it's used, *always* append to it
set(kdecore_OPTIONAL_SRCS)


# you can find macro_optional_find_package() in kdelibs/cmake/modules/OptionalFindPackage.cmake
# it is the same as FIND_PACKAGE(<name>) but additionally creates an OPTION(WITH_<name>)
# so the checking for the software can be disabled via ccmake or -DWITH_<name>=OFF
macro_optional_find_package(AGG)
if (AGG_FOUND)
   include_directories(${AGG_INCLUDE_DIR})
   set(kdecore_OPTIONAL_SRCS ${kdecore_OPTIONAL_SRCS} svgicons/ksvgiconengine.cpp svgicons/ksvgiconpainter.cpp )
	install_files( /include FILES svgicons/ksvgiconengine.h )
endif (AGG_FOUND)

# compile bzip2 support if available
if(BZIP2_FOUND)
  include_directories(${BZIP2_INCLUDE_DIR})
  set(kdecore_OPTIONAL_SRCS ${kdecore_OPTIONAL_SRCS} kbzip2filter.cpp )
endif(BZIP2_FOUND)

# compile Gettext support if available
if(GETTEXT_FOUND)
  include_directories(${GETTEXT_INCLUDE_DIR})
endif(GETTEXT_FOUND)

if (WIN32)
   set(kdecore_OPTIONAL_SRCS ${kdecore_OPTIONAL_SRCS}
      ${CMAKE_SOURCE_DIR}/libltdl/ltdl_win.c
      kapplication_win.cpp 
      kglobalaccel_win.cpp
      ktoolinvocation_win.cpp 
      #netsupp_win32.cpp is obsolete
      )
endif (WIN32)


if (UNIX)
   set(kdecore_OPTIONAL_SRCS ${kdecore_OPTIONAL_SRCS}
      kglobalaccel_x11.cpp 
      ktoolinvocation_x11.cpp 
      ksocks.cpp
      netsupp.cpp
      netwm.cpp
      kpty.cpp
      kwin.cpp
      kwinmodule.cpp 
      network/khttpproxysocketdevice.cpp
      network/ksockssocketdevice.cpp
      network/knetworkinterface.cpp )
endif (UNIX)


add_subdirectory( malloc )
add_subdirectory( kconfig_compiler )
add_subdirectory( tests )
add_subdirectory( network )


########### next target ###############

if (NOT WIN32)

   kde4_add_library(kdefakes SHARED fakes.c)
   set_target_properties(kdefakes PROPERTIES VERSION 5.0.0 SOVERSION 5 )
   install_targets(${LIB_INSTALL_DIR} kdefakes )

   #no ttyname on windows
   kde4_add_executable(kgrantpty kgrantpty.c)
   target_link_libraries(kgrantpty  kdefakes )
   install_targets(/bin kgrantpty )
   
endif (NOT WIN32)



########### next target ###############

set(kdecore_LIB_SRCS
   kapplication.cpp
   kauthorized.cpp
   kautostart.cpp
   kdebug.cpp
   kconfigbase.cpp
   kconfig.cpp
   ksimpleconfig.cpp
   kconfigbackend.cpp
   kmanagerselection.cpp
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
   kkeynative_x11.cpp
   kkeyserver_x11.cpp
   kaccelaction.cpp
   kshortcutmenu.cpp
   kaccelbase.cpp
   kaccel.cpp
   kglobalaccel.cpp
   kstdaccel.cpp
   kshortcutlist.cpp
   kcrash.cpp
   kurl.cpp
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
   kcompletion.cpp
   kcmdlineargs.cpp
   kaboutdata.cpp
   kcompletionbase.cpp
   knotifyclient.cpp
   kaudioplayer.cpp
   kdcoppropertyproxy.cpp
   kprocio.cpp
   krandom.cpp
   kpixmapprovider.cpp
   ktoolinvocation.cpp
   kcodecs.cpp
   ksycoca.cpp
   ksycocadict.cpp
   ksycocafactory.cpp
   kxmessages.cpp
   kstartupinfo.cpp
   kcatalog.cpp
   kstaticdeleter.cpp
   kappdcopiface.cpp
   kclipboard.cpp
   kcheckaccelerators.cpp
   kdeversion.cpp
   kdebugdcopiface.cpp
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
   knotificationrestrictions.cpp
   kprotocolinfo_kdecore.cpp
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
   malloc/malloc.c
   ${CMAKE_SOURCE_DIR}/libltdl/ltdl.c
)

kde4_automoc(${kdecore_LIB_SRCS})

set( kdecore_DCOP_SKEL_SRCS
   ksycoca.h
   kappdcopiface.h
   kdebugdcopiface.h
   kdedmodule.h
)

if(UNIX)
    set(kdecore_LIB_SRCS ${kdecore_LIB_SRCS} fakes.c)
endif(UNIX)

kde4_add_dcop_skels(kdecore_LIB_SRCS ${kdecore_DCOP_SKEL_SRCS})

kde4_add_library(kdecore SHARED ${kdecore_LIB_SRCS})

target_link_libraries(kdecore  ${QT_QTCORE_LIBRARY} ${QT_QT3SUPPORT_LIBRARY} ${QT_QTGUI_LIBRARY} ${X11_LIBRARIES} ${UTIL_LIBRARY} DCOP ${ZLIB_LIBRARY} ${CARBON_LIBRARY})
if (HAVE_RESOLV_LIBRARY)
  target_link_libraries(kdecore  resolv)
endif (HAVE_RESOLV_LIBRARY)

if (WIN32)
    add_definitions(-DKIO_EXPORT=)
endif (WIN32)

if(HAVE_IDNA_H)
   target_link_libraries(kdecore idn)
endif(HAVE_IDNA_H)

if(BZIP2_FOUND)
   target_link_libraries(kdecore  ${BZIP2_LIBRARIES})
endif(BZIP2_FOUND)

if(AGG_FOUND)
   target_link_libraries(kdecore  ${QT_QTXML_LIBRARY} ${AGG_LIBRARIES})
endif(AGG_FOUND)

if(GETTEXT_FOUND)
  target_link_libraries(kdecore  ${GETTEXT_LIBRARIES})
endif(GETTEXT_FOUND)

if(HAVE_LIBDL)
   target_link_libraries(kdecore dl)
endif(HAVE_LIBDL)

if(X11_XTest_FOUND)
   target_link_libraries(kdecore ${X11_XTest_LIB})
endif(X11_XTest_FOUND)

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


configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kde-config.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/kde-config.cpp @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/all_languages.desktop ${CMAKE_CURRENT_BINARY_DIR}/all_languages @ONLY)  

#macro_additional_clean_files( ${CMAKE_CURRENT_BINARY_DIR}/kde-config.cpp  ${CMAKE_CURRENT_BINARY_DIR}/all_languages )

kde4_add_executable(kde-config ${CMAKE_CURRENT_BINARY_DIR}/kde-config.cpp )

target_link_libraries(kde-config ${KDE4_KDECORE_LIBS} )

install_targets(/bin kde-config)


########### install files ###############
install_files( ${LOCALE_INSTALL_DIR} FILES all_languages )
install_files( ${DATA_INSTALL_DIR}/knotify FILES eventsrc )
install_files( ${CONFIG_INSTALL_DIR} FILES kdebug.areas kdebugrc language.codes )
install_files( ${CONFIG_INSTALL_DIR}/colors FILES 40.colors Web.colors Royal.colors Rainbow.colors )
install_files( /include FILES kconfig.h kconfigskeleton.h   kconfigdata.h ksimpleconfig.h    kconfigbase.h kdesktopfile.h kurl.h kaboutdata.h   kcmdlineargs.h kconfigbackend.h kapplication.h kauthorized.h ktoolinvocation.h   kuniqueapplication.h kcharsets.h kdeversion.h kpty.h kprocess.h   kprocctrl.h klocale.h klocalizedstring.h kicontheme.h kiconloader.h kdebug.h   kwinmodule.h kwin.h kshortcut.h kkeynative.h kaccel.h    kglobalaccel.h kstdaccel.h kshortcutlist.h kcatalog.h    kcompletion.h kstringhandler.h   kstandarddirs.h kglobal.h kglobalsettings.h ksharedptr.h    kallocator.h kvmallocator.h kcrash.h krfcdate.h    kinstance.h kpalette.h kipc.h klibloader.h ktempfile.h ksavefile.h   krandomsequence.h knotifyclient.h kiconeffect.h    kaudioplayer.h kdcoppropertyproxy.h netwm.h kacceleratormanager.h    netwm_def.h kpixmapprovider.h kstaticdeleter.h     kprocio.h kasyncio.h krandom.h   kbufferedio.h kmimesourcefactory.h kcodecs.h ksocks.h    ksycoca.h ksycocaentry.h ksycocatype.h kxmessages.h kstartupinfo.h   kgenericfactory.h kgenericfactory.tcc     ktypelist.h ksortablevaluelist.h kclipboard.h   kcalendarsystem.h kcalendarsystemfactory.h kmacroexpander.h    kmanagerselection.h kmountpoint.h kuser.h klockfile.h    kidna.h ktempdir.h kshell.h fixx11h.h kxerrorhandler.h   kde_file.h ktimezones.h ktzfiletimezone.h kfilterbase.h kfilterdev.h kdatetime.h qtest_kde.h conversion_check.h   kdedmodule.h kautostart.h knotificationrestrictions.h )

kde4_create_doxygen_docs( REFERENCES dcop kdeui kparts kio EXCLUDE malloc kde_file.h  )

kde4_footer()


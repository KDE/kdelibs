project(kdecore)

# Generate config-kstandarddirs.h
set( KDELIBSUFF ${LIB_SUFFIX} )

configure_file(config-kstandarddirs.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kstandarddirs.h )
configure_file(config-ksycoca.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-ksycoca.h )

find_package(X11)
if(X11_FOUND)
	macro_bool_to_01(X11_FOUND HAVE_X11)
endif(X11_FOUND)

find_package(Libintl)
macro_log_feature(LIBINTL_FOUND "Libintl" "Support for multiple languages" "http://www.gnu.org/software/gettext" FALSE "" "STRONGLY RECOMMENDED: Enables KDE to be available in many different languages")

# Generate kdefakes.h
include(ConfigureChecks.cmake)
configure_file(kdefakes.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdefakes.h )

# Generate kdeversion.h
configure_file(util/kdeversion.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdeversion.h )

check_struct_member("struct sockaddr" sa_len "sys/types.h;sys/socket.h" HAVE_STRUCT_SOCKADDR_SA_LEN)
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
IF(NOT QT5_BUILD)
    include_directories( ${QtMimeTypes_SOURCE_DIR}/include/QtMimeTypes )
endif()
include_directories( ${CMAKE_SOURCE_DIR}/tier1/kcoreaddons/src/jobs )
include_directories( ${CMAKE_SOURCE_DIR}/tier1/kcoreaddons/src/io )
include_directories( ${CMAKE_SOURCE_DIR}/tier1/kcodecs/src )
include_directories( ${CMAKE_SOURCE_DIR}/tier1/kcoreaddons/src/text )
include_directories( ${CMAKE_SOURCE_DIR}/tier1/kcoreaddons/src/kernel )
include_directories( ${CMAKE_SOURCE_DIR}/tier1/kcoreaddons/src/randomness )
include_directories( ${CMAKE_BINARY_DIR}/tier1/kcoreaddons/src/ )
include_directories( ${CMAKE_BINARY_DIR}/tier1/kcodecs/src/ )

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
endif (WIN32)


if (UNIX)
   set(kdecore_OPTIONAL_SRCS ${kdecore_OPTIONAL_SRCS}
      kernel/ktoolinvocation_x11.cpp
      localization/klocale_unix.cpp
      util/kshell_unix.cpp
      util/kuser_unix.cpp
      fakes.c
   )
endif (UNIX)

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
   io/kurl.cpp
   kernel/kglobal.cpp
   kernel/kcomponentdata.cpp
   kernel/kcmdlineargs.cpp
   kernel/kstandarddirs.cpp
   kernel/ktoolinvocation.cpp
   services/kmimetypefactory.cpp
   services/kmimetypetrader.cpp
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

set(kdecore_LIB_SRCS ${kdecore_LIB_SRCS}
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
   sycoca/kmemfile.cpp
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

if (NOT Q_OS_X11)
   add_definitions(-DNO_DISPLAY)
endif()

kde4_add_library(kdecore ${LIBRARY_TYPE} ${kdecore_LIB_SRCS})
generate_export_header(kdecore)

target_link_libraries(kdecore
  LINK_PUBLIC
    ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTDBUS_LIBRARY} kcoreaddons kconfigcore kcodecs
  LINK_PRIVATE
    ${QtMimeTypes_LIBRARY} ${QT_QTXML_LIBRARY} ${kdecore_OPTIONAL_LIBS}
)

if(WINCE)
  target_link_libraries(kdecore ${WCECOMPAT_LIBRARIES} Ceshell.lib)
endif(WINCE)

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
   io/kurl.h
   kernel/kglobal.h
   kernel/kcomponentdata.h
   kernel/kcmdlineargs.h
   kernel/kstandarddirs.h
   kernel/ktoolinvocation.h
   DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

install(FILES
   network/ksocketfactory.h
   network/ktcpsocket.h
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
   sycoca/ksycoca.h
   sycoca/ksycocaentry.h
   sycoca/ksycocatype.h
   util/kautostart.h
   util/kde_file.h
   util/kdedmodule.h
   util/klauncher_iface.h
   util/klibrary.h
   util/kexportplugin.h
   util/kpluginfactory.h
   util/kpluginloader.h
   util/kshell.h
   util/ksortablelist.h
   util/kuser.h
   util/qtest_kde.h
   ${CMAKE_CURRENT_BINARY_DIR}/kdefakes.h
   ${CMAKE_CURRENT_BINARY_DIR}/kdeversion.h
   DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

install( FILES
   #config/kconfigbackend.desktop
   services/kplugininfo.desktop
   DESTINATION  ${SERVICETYPES_INSTALL_DIR}
)

install( FILES
    KDECoreMacros.cmake
    DESTINATION ${DATA_INSTALL_DIR}/cmake/modules COMPONENT Devel )

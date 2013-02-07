project(kdecore)

find_package(X11)

# Generate config-kstandarddirs.h
set( KDELIBSUFF ${LIB_SUFFIX} )
configure_file(config-kstandarddirs.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kstandarddirs.h )

find_package(X11)
if(X11_FOUND)
	macro_bool_to_01(X11_FOUND HAVE_X11)
endif(X11_FOUND)

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

# Configure checks for io/
include(io/ConfigureChecks.cmake)
configure_file(io/config-io.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/io/config-io.h)
configure_file(io/config-kmountpoint.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/io/config-kmountpoint.h)
include_directories( ${CMAKE_CURRENT_BINARY_DIR}/io )

# Configure checks for kernel/
include(kernel/ConfigureChecks.cmake)
configure_file(kernel/config-kernel.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kernel/config-kernel.h)
include_directories( ${CMAKE_CURRENT_BINARY_DIR}/kernel )

# Configure checks for localization
include(localization/ConfigureChecks.cmake)
configure_file(localization/config-localization.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/localization/config-localization.h)
include_directories( ${CMAKE_CURRENT_BINARY_DIR}/localization )

include_directories( ${KDE4_KDECORE_INCLUDES} )
include_directories( ${ZLIB_INCLUDE_DIR} )
include_directories( ${QT_INCLUDES} )
include_directories( ${CMAKE_SOURCE_DIR}/tier1/kcoreaddons/src/jobs )
include_directories( ${CMAKE_SOURCE_DIR}/tier1/kcoreaddons/src/io )
include_directories( ${CMAKE_SOURCE_DIR}/tier1/kcodecs/src )
include_directories( ${CMAKE_SOURCE_DIR}/tier1/kcoreaddons/src/text )
include_directories( ${CMAKE_SOURCE_DIR}/tier1/kcoreaddons/src/kernel )
include_directories( ${CMAKE_SOURCE_DIR}/tier1/kcoreaddons/src/randomness )
include_directories( ${CMAKE_BINARY_DIR}/tier1/kcoreaddons/src/ )
include_directories( ${CMAKE_BINARY_DIR}/tier1/kcodecs/src/ )
include_directories( ${CMAKE_BINARY_DIR}/staging/kservice/src
                     ${CMAKE_SOURCE_DIR}/staging/kservice/src/services
                     ${CMAKE_SOURCE_DIR}/staging/kservice/src/sycoca
)
include_directories( ${ki18n_SOURCE_DIR}/src )
include_directories( ${ki18n_BINARY_DIR}/src )
include_directories( ${CMAKE_SOURCE_DIR}/staging/ki18n/src )
include_directories( ${CMAKE_BINARY_DIR}/staging/ki18n/src )

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
include_directories(${LIBINTL_INCLUDE_DIR})
set(kdecore_OPTIONAL_LIBS ${kdecore_OPTIONAL_LIBS} ${LIBINTL_LIBRARIES})

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
   install(TARGETS kdefakes EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

endif (UNIX)



########### next target ###############

set(kdecore_LIB_SRCS
   date/kcalendarera.cpp
   date/kcalendarsystem.cpp
   date/kcalendarsystemcoptic.cpp
   date/kcalendarsystemethiopian.cpp
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
   kernel/kcmdlineargs.cpp
   kernel/kstandarddirs.cpp
   kernel/ktoolinvocation.cpp
)

set(kdecore_LIB_SRCS ${kdecore_LIB_SRCS}
   network/ksocketfactory.cpp
   network/ksslcertificatemanager.cpp
   network/ktcpsocket.cpp
   localization/kcurrencycode.cpp
   localization/klocale.cpp
   localization/klocale_kde.cpp
   util/kautostart.cpp
   util/kdedmodule.cpp
   util/kdeversion.cpp
   util/klauncher_iface.cpp
   util/kshell.cpp
   ${kdecore_OPTIONAL_SRCS}
)

if (NOT HAVE_X11)
   add_definitions(-DNO_DISPLAY)
endif()

kde4_add_library(kdecore ${LIBRARY_TYPE} ${kdecore_LIB_SRCS})
generate_export_header(kdecore)

target_link_libraries(kdecore
  LINK_PUBLIC
    ${QT_QTNETWORK_LIBRARY} ${QT_QTDBUS_LIBRARY} kcoreaddons kconfigcore kcodecs kservice ki18n
  LINK_PRIVATE
    ${QT_QTXML_LIBRARY} ${kdecore_OPTIONAL_LIBS}
)

if(WINCE)
  target_link_libraries(kdecore ${WCECOMPAT_LIBRARIES} Ceshell.lib)
endif(WINCE)

if(WIN32)
  set(kdecore_LINK_INTERFACE_LIBRARIES  ${QT_QTDBUS_LIBRARY} ${KDEWIN_LIBRARIES})
else(WIN32)
  if(APPLE)
    set(kdecore_LINK_INTERFACE_LIBRARIES ${QT_QTDBUS_LIBRARY} ${CARBON_LIBRARY})
  else(APPLE)
    set(kdecore_LINK_INTERFACE_LIBRARIES ${QT_QTDBUS_LIBRARY} )
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

kde4_add_executable(kde5-config NOGUI kde-config.cpp )

target_link_libraries(kde5-config ${KDE4_KDECORE_LIBS} kcoreaddons)

install(TARGETS kde5-config ${INSTALL_TARGETS_DEFAULT_ARGS} )


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
   kernel/kcmdlineargs.h
   kernel/kstandarddirs.h
   kernel/ktoolinvocation.h
   DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

install(FILES
   network/ksocketfactory.h
   network/ktcpsocket.h
   localization/kcurrencycode.h
   localization/klocale.h
   util/kautostart.h
   util/kde_file.h
   util/kdedmodule.h
   util/klauncher_iface.h
   util/kshell.h
   util/ksortablelist.h
   util/kuser.h
   ${CMAKE_CURRENT_BINARY_DIR}/kdefakes.h
   ${CMAKE_CURRENT_BINARY_DIR}/kdeversion.h
   DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

#install( FILES
   #config/kconfigbackend.desktop
   #DESTINATION  ${SERVICETYPES_INSTALL_DIR}
   #)

install( FILES
    KDECoreMacros.cmake
    DESTINATION ${CMAKECONFIG_INSTALL_DIR} COMPONENT Devel )

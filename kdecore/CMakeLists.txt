project(kdecore)

find_package(X11)

find_package(Qt5PrintSupport 5.2.0 REQUIRED NO_MODULE)

find_package(X11)
if(X11_FOUND)
	macro_bool_to_01(X11_FOUND HAVE_X11)
endif(X11_FOUND)

include(ConfigureChecks.cmake)

# Generate kdeversion.h
configure_file(util/kdeversion.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdeversion.h )

check_struct_member("struct sockaddr" sa_len "sys/types.h;sys/socket.h" HAVE_STRUCT_SOCKADDR_SA_LEN)

# Configure checks for io/
include(io/ConfigureChecks.cmake)
configure_file(io/config-io.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/io/config-io.h)

# Configure checks for kernel/
configure_file(kernel/config-kernel.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kernel/config-kernel.h)


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
set(kdecore_OPTIONAL_LIBS ${kdecore_OPTIONAL_LIBS} ${LIBINTL_LIBRARIES})

if (HAVE_RESOLV_LIBRARY)
  set(kdecore_OPTIONAL_LIBS ${kdecore_OPTIONAL_LIBS} resolv)
endif (HAVE_RESOLV_LIBRARY)

if (WIN32)
   LIST(APPEND kdecore_OPTIONAL_SRCS
      util/kde_file_win.cpp
   )
endif (WIN32)

add_subdirectory( tests )

########### next target ###############

set(kdecore_LIB_SRCS
   io/kdebug.cpp
   io/kdebugdbusiface.cpp
   io/kprocess.cpp
)

set(kdecore_LIB_SRCS ${kdecore_LIB_SRCS}
   network/ksocketfactory.cpp
   util/kautostart.cpp
   util/kdeversion.cpp
   util/klauncher_iface.cpp
   ${kdecore_OPTIONAL_SRCS}
)

if (NOT HAVE_X11)
   add_definitions(-DNO_DISPLAY)
endif()

kde4_add_library(kdecore ${LIBRARY_TYPE} ${kdecore_LIB_SRCS})
generate_export_header(kdecore)


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
  # TODO: Move this definition to wherever kde_file.h goes.
  target_compile_definitions(kdecore PUBLIC -D_FILE_OFFSET_BITS=64)
endif ()


target_include_directories(kdecore PRIVATE
  ${ZLIB_INCLUDE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/network
  ${CMAKE_CURRENT_BINARY_DIR}/io
  ${CMAKE_CURRENT_BINARY_DIR}/kernel
  $<TARGET_PROPERTY:Qt5::PrintSupport,INTERFACE_INCLUDE_DIRECTORIES>
  ${LIBINTL_INCLUDE_DIR})

set(kdecore_INCLUDES
                          ${CMAKE_CURRENT_SOURCE_DIR}/io
                          ${CMAKE_CURRENT_SOURCE_DIR}/kernel
                          ${CMAKE_CURRENT_SOURCE_DIR}/network
                          ${CMAKE_CURRENT_SOURCE_DIR}/util
)
target_include_directories(kdecore PUBLIC "$<BUILD_INTERFACE:${kdecore_INCLUDES}>")

target_link_libraries(kdecore
  LINK_PUBLIC
    Qt5::Network Qt5::DBus ${KCoreAddons_LIBRARIES} ${KConfig_CORE_LIBRARY} ${KCodecs_LIBRARIES} KService KI18n
  LINK_PRIVATE
    Qt5::Xml KUnitConversion ${kdecore_OPTIONAL_LIBS} KDBusAddons
)

if(WINCE)
  target_link_libraries(kdecore ${WCECOMPAT_LIBRARIES} Ceshell.lib)
endif(WINCE)

if(WIN32)
  set(kdecore_LINK_INTERFACE_LIBRARIES  Qt5::DBus ${KDEWIN_LIBRARIES})
else(WIN32)
  if(APPLE)
    set(kdecore_LINK_INTERFACE_LIBRARIES Qt5::DBus ${CARBON_LIBRARY})
  else(APPLE)
    set(kdecore_LINK_INTERFACE_LIBRARIES Qt5::DBus )
  endif(APPLE)
endif(WIN32)

target_link_libraries(kdecore LINK_INTERFACE_LIBRARIES ${kdecore_LINK_INTERFACE_LIBRARIES} )

set_target_properties(kdecore PROPERTIES
                              VERSION ${KDE_NON_GENERIC_LIB_VERSION}
                              SOVERSION ${KDE_NON_GENERIC_LIB_SOVERSION}
                     )


install(TARGETS kdecore EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

########### next target ###############

########### install files ###############
install( FILES kdebug.areas kdebugrc DESTINATION ${CONFIG_INSTALL_DIR} )
install( FILES
   ${CMAKE_CURRENT_BINARY_DIR}/kdecore_export.h
   io/kdebug.h
   io/kprocess.h

   DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

install(FILES
   network/ksocketfactory.h
   util/kautostart.h
   util/kde_file.h
   util/klauncher_iface.h
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

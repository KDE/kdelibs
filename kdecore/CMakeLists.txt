project(kdecore)

find_package(X11)

find_package(Qt5PrintSupport 5.2.0 REQUIRED NO_MODULE)

find_package(X11)
if(X11_FOUND)
	macro_bool_to_01(X11_FOUND HAVE_X11)
endif(X11_FOUND)

include(ConfigureChecks.cmake)

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

########### next target ###############

set(kdecore_LIB_SRCS ${kdecore_LIB_SRCS}
   dummy.cpp
   ${kdecore_OPTIONAL_SRCS}
)

if (NOT HAVE_X11)
   add_definitions(-DNO_DISPLAY)
endif()

kde4_add_library(kdecore ${LIBRARY_TYPE} ${kdecore_LIB_SRCS})
generate_export_header(kdecore)






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

target_link_libraries(kdecore LINK_PUBLIC ${kdecore_LINK_INTERFACE_LIBRARIES} )

set_target_properties(kdecore PROPERTIES
                              VERSION ${KDE_NON_GENERIC_LIB_VERSION}
                              SOVERSION ${KDE_NON_GENERIC_LIB_SOVERSION}
                     )


install(TARGETS kdecore EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

########### next target ###############

########### install files ###############
install( FILES
   ${CMAKE_CURRENT_BINARY_DIR}/kdecore_export.h

   DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

#install( FILES
   #config/kconfigbackend.desktop
   #DESTINATION  ${SERVICETYPES_INSTALL_DIR}
   #)

install( FILES
    KDECoreMacros.cmake
    DESTINATION ${CMAKECONFIG_INSTALL_DIR} COMPONENT Devel )

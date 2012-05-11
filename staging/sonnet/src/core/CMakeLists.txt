project(sonnetcore)

include_directories(
  ${QT_INCLUDES}
  ${CMAKE_SOURCE_DIR}/kdecore/kernel
  ${CMAKE_SOURCE_DIR}/kdecore/util
  ${CMAKE_SOURCE_DIR}/kdecore/localization
  ${CMAKE_SOURCE_DIR}/kdecore/config
  ${CMAKE_SOURCE_DIR}/kdecore/global
  ${CMAKE_SOURCE_DIR}/kdecore/services
  ${CMAKE_SOURCE_DIR}/kdecore/sycoca
  ${CMAKE_SOURCE_DIR}/kdecore/io
  ${kcoreaddons_SOURCE_DIR}/src/io
  ${kcoreaddons_BINARY_DIR}/src
  ${kconfig_SOURCE_DIR}/src/core # ksharedptr.h, used by kservice
  ${kconfig_BINARY_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/kdecore
  ${CMAKE_BINARY_DIR}/kdecore
  ${CMAKE_CURRENT_BINARY_DIR}
)

include_directories( ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} )

set(sonnetcore_SRCS
   loader.cpp
   client.cpp
   spellerplugin.cpp
   speller.cpp
   filter.cpp
   settings.cpp
   backgroundchecker.cpp
   backgroundengine.cpp
   globals.cpp
)

add_library(sonnetcore ${LIBRARY_TYPE} ${sonnetcore_SRCS})
generate_export_header(sonnetcore)

target_link_libraries(sonnetcore ${QT_QTCORE_LIBRARY} ${KDE4_KDECORE_LIBS})

set_target_properties(sonnetcore PROPERTIES VERSION ${GENERIC_LIB_VERSION}
                                          SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(TARGETS sonnetcore EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})


install( FILES backgroundchecker.h speller.h globals.h ${CMAKE_CURRENT_BINARY_DIR}/sonnetcore_export.h
         DESTINATION ${INCLUDE_INSTALL_DIR}/sonnet COMPONENT Devel)


install( FILES
   sonnetspeller.desktop
   DESTINATION  ${SERVICETYPES_INSTALL_DIR}
)


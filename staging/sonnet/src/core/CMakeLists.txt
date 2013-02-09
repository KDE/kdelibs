project(sonnetcore)

include_directories(
  ${QT_INCLUDES}
  ${CMAKE_SOURCE_DIR}/kdecore/localization # TODO port to i18n framework
  ${CMAKE_BINARY_DIR}/kdecore

  ${kcoreaddons_SOURCE_DIR}/src/io
  ${kcoreaddons_BINARY_DIR}/src
  ${kconfig_SOURCE_DIR}/src/core
  ${kconfig_BINARY_DIR}/src/core
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

add_library(sonnetcore ${sonnetcore_SRCS})
generate_export_header(sonnetcore)

# TODO: port away from KDE4_KDECORE_LIBS
target_link_libraries(sonnetcore ${KDE4_KDECORE_LIBS})

set_target_properties(sonnetcore PROPERTIES VERSION ${ECM_VERSION_STRING}
                                          SOVERSION ${ECM_SOVERSION}
)

install(TARGETS sonnetcore EXPORT kdelibsLibraryTargets ${ECM_TARGET_DEFAULT_ARGS})


install( FILES backgroundchecker.h speller.h globals.h ${CMAKE_CURRENT_BINARY_DIR}/sonnetcore_export.h
         DESTINATION ${INCLUDE_INSTALL_DIR}/sonnet COMPONENT Devel)


install( FILES
   sonnetspeller.desktop
   DESTINATION  ${SERVICETYPES_INSTALL_DIR}
)


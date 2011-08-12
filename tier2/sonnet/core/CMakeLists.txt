project(sonnetcore)

include_directories(
  ${QT_INCLUDES}
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../kdecore/kernel
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../kdecore/util
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../kdecore/localization
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../kdecore/config
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../kdecore/global
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../kdecore/services
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../kdecore/sycoca
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../kdecore/io
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../kdecore
  ${CMAKE_CURRENT_BINARY_DIR}/../../../kdecore
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

kf5_add_library(sonnetcore ${LIBRARY_TYPE} ${sonnetcore_SRCS})

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

# add tests
add_subdirectory (tests)

project(sonnetcore)

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
set(sonnetcore_HEADERS
    backgroundchecker.h
    speller.h
    globals.h
    ${CMAKE_CURRENT_BINARY_DIR}/sonnetcore_export.h
)


add_library(SonnetCore ${sonnetcore_SRCS})
generate_export_header(SonnetCore)

target_link_libraries(SonnetCore LINK_PUBLIC Qt5::Core)

set_target_properties(SonnetCore PROPERTIES VERSION ${SONNET_VERSION_STRING}
                                          SOVERSION ${SONNET_SOVERSION}
)

install(TARGETS SonnetCore EXPORT SonnetTargets ${INSTALL_TARGETS_DEFAULT_ARGS})


install( FILES ${sonnetcore_HEADERS}
         DESTINATION ${INCLUDE_INSTALL_DIR}/sonnet COMPONENT Devel)


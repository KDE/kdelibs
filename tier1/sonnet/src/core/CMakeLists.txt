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


add_library(sonnetcore ${sonnetcore_SRCS})
generate_export_header(sonnetcore)

target_link_libraries(sonnetcore LINK_PUBLIC ${QT_QTCORE_LIBRARY})

set_target_properties(sonnetcore PROPERTIES VERSION ${ECM_VERSION_STRING}
                                          SOVERSION ${ECM_SOVERSION}
)

install(TARGETS sonnetcore EXPORT kdelibsLibraryTargets ${ECM_TARGET_DEFAULT_ARGS})


install( FILES ${sonnetcore_HEADERS}
         DESTINATION ${INCLUDE_INSTALL_DIR}/sonnet COMPONENT Devel)


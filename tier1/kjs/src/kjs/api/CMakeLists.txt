set(kjsapi_LIB_SRCS
    kjsinterpreter.cpp
    kjscontext.cpp
    kjsobject.cpp
    kjsprototype.cpp
    kjsarguments.cpp
   )

add_library(KJSApi ${kjsapi_LIB_SRCS})
generate_export_header(KJSApi)
add_library(KF5::KJSApi ALIAS KJSApi)

target_link_libraries(KJSApi
    LINK_PUBLIC Qt5::Core
    LINK_PRIVATE KJS)

set_target_properties(KJSApi PROPERTIES VERSION ${KJS_VERSION_STRING} SOVERSION ${KJS_SOVERSION} )
install(TARGETS KJSApi EXPORT KJSTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/kjsapi_export.h
        kjsinterpreter.h
        kjscontext.h
        kjsobject.h
        kjsprototype.h
        kjsarguments.h
        DESTINATION  ${INCLUDE_INSTALL_DIR}/kjs)

set(kjsapi_LIB_SRCS
    kjsinterpreter.cpp
    kjscontext.cpp
    kjsobject.cpp
    kjsprototype.cpp
    kjsarguments.cpp
   )

add_library(kjsapi ${kjsapi_LIB_SRCS})
generate_export_header(kjsapi)

target_link_libraries(kjsapi ${KJSLIBNAME} ${QT_QTCORE_LIBRARY})

set_target_properties(kjsapi PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kjsapi EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/kjsapi_export.h
        kjsinterpreter.h
        kjscontext.h
        kjsobject.h
        kjsprototype.h
        kjsarguments.h
        DESTINATION  ${INCLUDE_INSTALL_DIR}/kjs)

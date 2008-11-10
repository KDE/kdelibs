
include_directories(${QT_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} 
                    ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR}/Weaver )


set(ThreadWeaverExperimental_LIB_SRCS FileLoaderJob.cpp )

include_directories( ../Weaver )

kde4_add_library(ThreadWeaverExperimental SHARED ${ThreadWeaverExperimental_LIB_SRCS})
target_link_libraries(ThreadWeaverExperimental threadweaver)
set_target_properties(ThreadWeaverExperimental PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})

install(TARGETS ThreadWeaverExperimental EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES FileLoaderJob.h
        DESTINATION ${INCLUDE_INSTALL_DIR}/threadweaver/ COMPONENT Devel )


include_directories(${QT_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ../Weaver
                    ${CMAKE_CURRENT_BINARY_DIR}/../Weaver)


set(ThreadWeaverExperimental_LIB_SRCS FileLoaderJob.cpp )


add_library(ThreadWeaverExperimental ${LIBRARY_TYPE} ${ThreadWeaverExperimental_LIB_SRCS})
target_link_libraries(ThreadWeaverExperimental threadweaver ${QT_QTCORE_LIBRARY})
set_target_properties(ThreadWeaverExperimental PROPERTIES VERSION   ${ECM_VERSION_STRING}
                                                          SOVERSION ${ECM_SOVERSION})

install(TARGETS ThreadWeaverExperimental EXPORT kdelibsLibraryTargets ${ECM_TARGET_DEFAULT_ARGS})

install(FILES FileLoaderJob.h
        DESTINATION ${INCLUDE_INSTALL_DIR}/threadweaver/ COMPONENT Devel )

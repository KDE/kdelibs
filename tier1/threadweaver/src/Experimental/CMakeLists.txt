
set(ThreadWeaverExperimental_LIB_SRCS FileLoaderJob.cpp )


add_library(ThreadWeaverExperimental ${ThreadWeaverExperimental_LIB_SRCS})
generate_export_header(ThreadWeaverExperimental)
target_link_libraries(ThreadWeaverExperimental ThreadWeaver)
set_target_properties(ThreadWeaverExperimental PROPERTIES VERSION   ${THREADWEAVER_VERSION_STRING}
                                                          SOVERSION ${THREADWEAVER_SOVERSION})

install(TARGETS ThreadWeaverExperimental EXPORT ThreadWeaverTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES FileLoaderJob.h
        DESTINATION ${INCLUDE_INSTALL_DIR}/threadweaver/ COMPONENT Devel )

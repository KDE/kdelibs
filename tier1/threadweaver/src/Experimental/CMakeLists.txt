
include_directories(${QT_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ../Weaver
                    ${CMAKE_CURRENT_BINARY_DIR}/../Weaver)


set(ThreadWeaverExperimental_LIB_SRCS FileLoaderJob.cpp )


add_library(ThreadWeaverExperimental ${ThreadWeaverExperimental_LIB_SRCS})
generate_export_header(ThreadWeaverExperimental)
target_link_libraries(ThreadWeaverExperimental ThreadWeaver)
set_target_properties(ThreadWeaverExperimental PROPERTIES VERSION   ${THREADWEAVER_VERSION_STRING}
                                                          SOVERSION ${THREADWEAVER_SOVERSION})

install(TARGETS ThreadWeaverExperimental EXPORT ThreadWeaverTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES FileLoaderJob.h
        DESTINATION ${INCLUDE_INSTALL_DIR}/threadweaver/ COMPONENT Devel )

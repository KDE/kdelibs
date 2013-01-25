

set(ThreadWeaverGui_LIB_SRCS  WeaverThreadGrid.cpp )

find_package(Qt5Transitional REQUIRED MODULE COMPONENTS Widgets)

add_library(ThreadWeaverGui ${ThreadWeaverGui_LIB_SRCS})
generate_export_header(ThreadWeaverGui)

target_link_libraries(ThreadWeaverGui ThreadWeaver Qt5::Widgets)
set_target_properties(ThreadWeaverGui PROPERTIES VERSION   ${THREADWEAVER_VERSION_STRING}
                                                 SOVERSION ${THREADWEAVER_SOVERSION})

install(TARGETS ThreadWeaverGui EXPORT ThreadWeaverTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES WeaverThreadGrid.h
        DESTINATION ${INCLUDE_INSTALL_DIR}/threadweaver/ COMPONENT Devel
)

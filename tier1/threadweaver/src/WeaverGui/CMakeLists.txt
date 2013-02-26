

set(ThreadWeaverGui_LIB_SRCS  WeaverThreadGrid.cpp )

find_package(Qt5Transitional REQUIRED MODULE COMPONENTS Widgets)

include_directories(${QT_INCLUDES}
                    ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
                    ../Weaver ${CMAKE_CURRENT_BINARY_DIR}/../Weaver)

add_library(ThreadWeaverGui ${ThreadWeaverGui_LIB_SRCS})
generate_export_header(ThreadWeaverGui)

target_link_libraries(ThreadWeaverGui ThreadWeaver Qt5::Widgets)
set_target_properties(ThreadWeaverGui PROPERTIES VERSION   ${THREADWEAVER_VERSION_STRING}
                                                 SOVERSION ${THREADWEAVER_SOVERSION})

install(TARGETS ThreadWeaverGui EXPORT ThreadWeaverTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES WeaverThreadGrid.h
        DESTINATION ${INCLUDE_INSTALL_DIR}/threadweaver/ COMPONENT Devel
)

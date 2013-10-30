

set(ThreadWeaverGui_LIB_SRCS  WeaverThreadGrid.cpp )

find_package(Qt5Widgets 5.2.0 REQUIRED NO_MODULE)

add_library(ThreadWeaverGui ${ThreadWeaverGui_LIB_SRCS})
generate_export_header(ThreadWeaverGui)
add_library(KF5::ThreadWeaverGui ALIAS ThreadWeaverGui)
target_include_directories(ThreadWeaverGui
  INTERFACE
    "$<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}/threadweaver>"
)

target_link_libraries(ThreadWeaverGui PUBLIC ThreadWeaver Qt5::Widgets)
set_target_properties(ThreadWeaverGui PROPERTIES VERSION   ${THREADWEAVER_VERSION_STRING}
                                                 SOVERSION ${THREADWEAVER_SOVERSION})

install(TARGETS ThreadWeaverGui EXPORT ThreadWeaverTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES WeaverThreadGrid.h
        DESTINATION ${INCLUDE_INSTALL_DIR}/threadweaver/ COMPONENT Devel
)

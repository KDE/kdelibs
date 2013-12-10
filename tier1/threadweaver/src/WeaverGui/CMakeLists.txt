

set(ThreadWeaverGui_LIB_SRCS  WeaverThreadGrid.cpp )

find_package(Qt5Widgets 5.2.0 REQUIRED NO_MODULE)

add_library(KF5ThreadWeaverGui ${ThreadWeaverGui_LIB_SRCS})
generate_export_header(KF5ThreadWeaverGui BASE_NAME ThreadWeaverGui)
add_library(KF5::ThreadWeaverGui ALIAS KF5ThreadWeaverGui)
target_include_directories(KF5ThreadWeaverGui
  INTERFACE
    "$<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}/threadweaver>"
)

target_link_libraries(KF5ThreadWeaverGui PUBLIC KF5::ThreadWeaver Qt5::Widgets)
set_target_properties(KF5ThreadWeaverGui PROPERTIES VERSION   ${THREADWEAVER_VERSION_STRING}
                                                    SOVERSION ${THREADWEAVER_SOVERSION}
                                                    EXPORT_NAME ThreadWeaver
)

install(TARGETS KF5ThreadWeaverGui EXPORT KF5ThreadWeaverTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES WeaverThreadGrid.h
        DESTINATION ${INCLUDE_INSTALL_DIR}/threadweaver/ COMPONENT Devel
)

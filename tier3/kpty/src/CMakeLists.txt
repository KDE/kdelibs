set(kpty_LIB_SRCS
   kpty.cpp
   kptydevice.cpp
   kptyprocess.cpp
)

add_library(KF5Pty ${kpty_LIB_SRCS})
generate_export_header(KF5Pty BASE_NAME KPty)
add_library(KF5::Pty ALIAS KF5Pty)

include(ConfigureChecks.cmake)
configure_file(config-pty.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-pty.h )

target_link_libraries(KF5Pty PUBLIC Qt5::Core
                                    KF5::CoreAddons # KProcess
                             PRIVATE
                                    ${UTIL_LIBRARY}
                                    ${UTEMPTER_LIBRARY}
                                    KF5::I18n)

target_include_directories(KF5Pty PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/..)

set_target_properties(KF5Pty PROPERTIES
   VERSION ${KPTY_VERSION_STRING}
   SOVERSION ${KPTY_SOVERSION}
   EXPORT_NAME Pty
)
install(TARGETS KF5Pty EXPORT KF5PtyTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES
   ${CMAKE_CURRENT_BINARY_DIR}/kpty_export.h
   kpty.h
   kptydevice.h
   kptyprocess.h
   DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

########### next target ###############

if (NOT HAVE_OPENPTY)
   add_executable(kgrantpty kgrantpty.c)
   ecm_mark_nongui_executable(kgrantpty)
   install(TARGETS kgrantpty DESTINATION ${LIBEXEC_INSTALL_DIR})
endif ()

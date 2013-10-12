set(kpty_LIB_SRCS
   kpty.cpp
   kptydevice.cpp
   kptyprocess.cpp
)

add_library(KPty ${kpty_LIB_SRCS})
generate_export_header(KPty)
add_library(KF5::KPty ALIAS KPty)

include(ConfigureChecks.cmake)
configure_file(config-pty.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-pty.h )

target_link_libraries(KPty LINK_PUBLIC Qt5::Core
                                       KF5::KCoreAddons # KProcess
                           LINK_PRIVATE
                                ${UTIL_LIBRARY}
                                ${UTEMPTER_LIBRARY}
                                KF5::KI18n)

target_include_directories(KPty PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/..)

set_target_properties(KPty PROPERTIES
   VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
)
install(TARGETS KPty EXPORT KPtyTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

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
endif (NOT HAVE_OPENPTY)

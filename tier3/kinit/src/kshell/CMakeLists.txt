add_executable(kshell5 shell.cpp)
ecm_mark_nongui_executable(kshell5)

target_link_libraries(kshell5 ${KINIT_SOCKET_LIBRARY} Qt5::Core)

install(TARGETS kshell5 ${INSTALL_TARGETS_DEFAULT_ARGS} )

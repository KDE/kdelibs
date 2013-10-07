add_executable(start_kdeinit start_kdeinit.c)
target_include_directories(start_kdeinit PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/..")

add_executable(start_kdeinit_wrapper start_kdeinit_wrapper.c)
target_include_directories(start_kdeinit_wrapper PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/..")

install(TARGETS start_kdeinit DESTINATION ${LIBEXEC_INSTALL_DIR})
install(TARGETS start_kdeinit_wrapper DESTINATION ${LIBEXEC_INSTALL_DIR})

if (CMAKE_SYSTEM_NAME MATCHES Linux)
    MESSAGE(STATUS "Using setuid root kdeinit wrapper in order to protect it from bad Linux OOM-killer")
    set(KDEINIT_OOM_PROTECT 1)
    install(CODE "
      set(START_KDEINIT_PATH \"\$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/${LIBEXEC_INSTALL_DIR}/start_kdeinit\")
        EXECUTE_PROCESS(COMMAND sh -c \"chown 0 '\${START_KDEINIT_PATH}' && chmod u+s '\${START_KDEINIT_PATH}'\")
    ")
endif (CMAKE_SYSTEM_NAME MATCHES Linux)

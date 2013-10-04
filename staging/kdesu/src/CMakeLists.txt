include(CheckTypeSize)

check_function_exists(setpriority HAVE_SETPRIORITY)
check_function_exists(getpeereid HAVE_GETPEEREID)
check_function_exists(initgroups HAVE_INITGROUPS)

check_include_files(sys/select.h  HAVE_SYS_SELECT_H)

set(CMAKE_EXTRA_INCLUDE_FILES sys/socket.h)
check_type_size("struct ucred" STRUCT_UCRED) #defines HAVE_STRUCT_UCRED (bool) and STRUCT_UCRED (size, unused)
set(CMAKE_EXTRA_INCLUDE_FILES)  #reset CMAKE_EXTRA_INCLUDE_FILES

configure_file(config-kdesu.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kdesu.h)

set(kdesu_LIB_SRCS
  client.cpp
  process.cpp
  kcookie.cpp
  su.cpp
  ssh.cpp
  stub.cpp
)


add_library(KDESu ${kdesu_LIB_SRCS})
generate_export_header(KDESu)

target_include_directories(KDESu PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/..)

target_link_libraries(KDESu
  LINK_PUBLIC
    KPty
  LINK_PRIVATE
    KCoreAddons # KUser::loginName
    KService # KToolInvocation::kdeinitExecWait
)

if(KDESU_USE_SUDO_DEFAULT)
   set_source_files_properties(su.cpp PROPERTIES 
                               COMPILE_FLAGS -DKDESU_USE_SUDO_DEFAULT)
endif(KDESU_USE_SUDO_DEFAULT)

set_target_properties(KDESu PROPERTIES
  VERSION ${KDESU_VERSION_STRING}
  SOVERSION ${KDESU_SOVERSION}
)

install(TARGETS KDESu EXPORT KDESuTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

########### next target ###############

add_executable(kdesu_stub kdesu_stub.c)
ecm_mark_nongui_executable(kdesu_stub)

install(TARGETS kdesu_stub DESTINATION ${LIBEXEC_INSTALL_DIR})

########### install files ###############
install(FILES
  client.h
  process.h
  su.h
  ssh.h
  stub.h
  defaults.h
  ${CMAKE_CURRENT_BINARY_DIR}/kdesu_export.h
  DESTINATION  ${INCLUDE_INSTALL_DIR}/kdesu COMPONENT Devel
)

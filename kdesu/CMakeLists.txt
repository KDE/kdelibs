project(kdesu)

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include(CheckTypeSize)

check_function_exists(setpriority HAVE_SETPRIORITY)
check_function_exists(getpeereid HAVE_GETPEEREID)
check_function_exists(initgroups HAVE_INITGROUPS)

check_include_files(sys/select.h  HAVE_SYS_SELECT_H)

#optional features
find_package(X11)
if(X11_FOUND)
	macro_bool_to_01(X11_FOUND HAVE_X11)
endif(X11_FOUND)


set(CMAKE_EXTRA_INCLUDE_FILES sys/socket.h)
check_type_size("struct ucred" STRUCT_UCRED) #defines HAVE_STRUCT_UCRED (bool) and STRUCT_UCRED (size, unused)
set(CMAKE_EXTRA_INCLUDE_FILES)  #reset CMAKE_EXTRA_INCLUDE_FILES

configure_file(config-kdesu.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kdesu.h )

########### next target ###############

set(kdesu_LIB_SRCS
   client.cpp
   process.cpp
   kcookie.cpp
   su.cpp
   ssh.cpp
   stub.cpp
   )


add_library(kdesu ${LIBRARY_TYPE} ${kdesu_LIB_SRCS})
generate_export_header(kdesu)

target_include_directories(kdesu PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/..)

target_link_libraries(kdesu
  LINK_PUBLIC
    ${KDE4_KPTY_LIBS}
  LINK_PRIVATE
    KCoreAddons
    KI18n
    KService # KToolInvocation::kdeinitExecWait
)

if(KDESU_USE_SUDO_DEFAULT)
   set_source_files_properties(su.cpp PROPERTIES 
                               COMPILE_FLAGS -DKDESU_USE_SUDO_DEFAULT)
endif(KDESU_USE_SUDO_DEFAULT)

set_target_properties(kdesu PROPERTIES
   VERSION ${KDE_NON_GENERIC_LIB_VERSION}
   SOVERSION ${KDE_NON_GENERIC_LIB_SOVERSION}
)

install(TARGETS kdesu EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### next target ###############

add_executable(kdesu_stub kdesu_stub.c)
ecm_mark_nongui_executable(kdesu_stub)

install(TARGETS kdesu_stub DESTINATION ${LIBEXEC_INSTALL_DIR} )


########### install files ###############
install( FILES ${CMAKE_CURRENT_BINARY_DIR}/kdesu_export.h client.h process.h su.h ssh.h stub.h defaults.h
         DESTINATION  ${INCLUDE_INSTALL_DIR}/kdesu COMPONENT Devel)





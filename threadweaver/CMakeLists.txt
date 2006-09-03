project(threadweaver)
#add some KDE specific stuff
set(LIB_SUFFIX "" CACHE STRING "Define suffix of directory name (32/64)" )
 set(SHARE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}/share CACHE PATH "Base directory for files which go to share/" FORCE)
 set(EXEC_INSTALL_PREFIX  ${CMAKE_INSTALL_PREFIX}       CACHE PATH  "Base directory for executables and libraries" FORCE)
#
## the following are directories where stuff will be installed to
 set(BIN_INSTALL_DIR          "${EXEC_INSTALL_PREFIX}/bin"                  CACHE PATH "The kde info install dir (default prefix/info)" FORCE)
 set(SBIN_INSTALL_DIR         "${EXEC_INSTALL_PREFIX}/sbin"                 CACHE PATH "The kde info install dir (default prefix/info)" FORCE)
 set(LIB_INSTALL_DIR          "${EXEC_INSTALL_PREFIX}/lib${LIB_SUFFIX}"     CACHE PATH "The subdirectory relative to the install prefix where libraries will be installed (default is /lib${LIB_SUFFIX})" FORCE)
 set(LIBEXEC_INSTALL_DIR      "${LIB_INSTALL_DIR}/kde4/libexec"             CACHE PATH "The subdirectory relative to the install prefix where libraries will be installed (default is /lib/kde4/libexec)" FORCE)
 set(PLUGIN_INSTALL_DIR       "${LIB_INSTALL_DIR}/kde4"                     CACHE PATH "The subdirectory relative to the install prefix where plugins will be installed (default is ${KDE4_LIB_INSTALL_DIR}/kde4)" FORCE)
 set(INCLUDE_INSTALL_DIR      "${CMAKE_INSTALL_PREFIX}/include"             CACHE PATH "The subdirectory to the header prefix" FORCE)

#find_package(Qt4 REQUIRED)
include (FindQt4)

add_subdirectory(Weaver)
add_subdirectory(WeaverGui)
add_subdirectory(Action)
add_subdirectory(Experimental)
add_subdirectory(Tests)
add_subdirectory(Examples)

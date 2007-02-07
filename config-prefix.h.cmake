/* This file contains all the paths that change when changing the installation prefix */

#define KDEDIR "${CMAKE_INSTALL_PREFIX}"

/* The compiled in system configuration prefix */
#define KDESYSCONFDIR "${SYSCONF_INSTALL_DIR}"

#define __KDE_BINDIR "${BIN_INSTALL_DIR}"

/* Compile in the exec prefix to help kstddirs in finding dynamic libs
   (This was for exec_prefix != prefix - still needed?) */
#define __KDE_EXECPREFIX "NONE"

#define LIBEXEC_INSTALL_DIR "${LIBEXEC_INSTALL_DIR}"


/* This file contains all the paths that change when changing the installation prefix */

#define KDEDIR "${CMAKE_INSTALL_PREFIX}"

/* The compiled in system configuration prefix */
#define KDESYSCONFDIR "${CMAKE_INSTALL_PREFIX}/etc"

#define __KDE_BINDIR "${CMAKE_INSTALL_PREFIX}/bin"

#define DCOP_PATH "${CMAKE_INSTALL_PREFIX}/bin"

/* Compile in the exec prefix to help kstddirs in finding dynamic libs
   (This was for exec_prefix != prefix - still needed?) */
#define __KDE_EXECPREFIX "NONE"


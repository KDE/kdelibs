#define kde_socklen_t socklen_t

#cmakedefine01 HAVE_X11
#cmakedefine01 HAVE_SYS_STAT_H
#cmakedefine01 HAVE_PATHS_H
#cmakedefine01 HAVE_LIMITS_H
#cmakedefine01 HAVE_XTEST
#cmakedefine01 HAVE_MADVISE 
#cmakedefine01 HAVE_SYS_TYPES_H
#cmakedefine01 HAVE_SYS_TIME_H
#cmakedefine01 HAVE_UNISTD_H
/* Define to 1 if you have the Xrender library */
#cmakedefine01 HAVE_XRENDER
#cmakedefine01 HAVE_DBUSMENUQT

#define KDE_COMPILER_VERSION "${KDE_COMPILER_VERSION}"
#define KDE_COMPILING_OS  "${CMAKE_SYSTEM}"
#define KDE_DISTRIBUTION_TEXT "${KDE_DISTRIBUTION_TEXT}"

#define CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}"
#define LIBEXEC_INSTALL_DIR "${LIBEXEC_INSTALL_DIR}"

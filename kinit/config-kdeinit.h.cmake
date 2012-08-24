#define kde_socklen_t socklen_t
#define KDELIBSUFF "${KDELIBSUFF}"

/* These are for proctitle.cpp: */
#cmakedefine01 HAVE___PROGNAME
#cmakedefine01 HAVE___PROGNAME_FULL
#cmakedefine01 HAVE_SYS_PSTAT_H
#cmakedefine01 HAVE_PSTAT
#cmakedefine01 HAVE_SETPROCTITLE
#cmakedefine01 HAVE_LIMITS_H
#cmakedefine01 HAVE_X11
#cmakedefine01 HAVE_SYS_SELECT_H
#cmakedefine01 HAVE_SYS_EXEC_H

/* for start_kdeinit */
#cmakedefine01 KDEINIT_OOM_PROTECT


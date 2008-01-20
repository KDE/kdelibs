/* Defined to the path of the PTY multiplexer device, if any */
#cmakedefine PTM_DEVICE "${PTM_DEVICE}"

#cmakedefine HAVE_POSIX_OPENPT 1
#cmakedefine HAVE_GETPT 1
#cmakedefine HAVE_GRANTPT 1
#cmakedefine HAVE_OPENPTY 1
#cmakedefine HAVE_PTSNAME 1
#cmakedefine HAVE_REVOKE 1
#cmakedefine HAVE_UNLOCKPT 1
#cmakedefine HAVE__GETPTY 1

#cmakedefine HAVE_LIBUTIL_H 1
#cmakedefine HAVE_UTIL_H 1
#cmakedefine HAVE_PTY_H 1
#cmakedefine HAVE_TERMIOS_H 1
#cmakedefine HAVE_TERMIO_H 1
#cmakedefine HAVE_SYS_STROPTS_H 1
#cmakedefine HAVE_SYS_FILIO_H 1

#cmakedefine HAVE_UTEMPTER 1
#cmakedefine HAVE_LOGIN 1
#cmakedefine HAVE_UTMPX 1
#cmakedefine HAVE_LOGINX 1
#cmakedefine HAVE_STRUCT_UTMP_UT_TYPE 1
#cmakedefine HAVE_STRUCT_UTMP_UT_PID 1
#cmakedefine HAVE_STRUCT_UTMP_UT_SESSION 1
#cmakedefine HAVE_STRUCT_UTMP_UT_SYSLEN 1
#cmakedefine HAVE_STRUCT_UTMP_UT_ID 1

/*
 * Steven Schultz <sms at to.gd-es.com> tells us :
 * BSD/OS 4.2 doesn't have a prototype for openpty in its system header files
 */
#ifdef __bsdi__
__BEGIN_DECLS
int openpty(int *, int *, char *, struct termios *, struct winsize *);
__END_DECLS
#endif


/*
 * Copyright (c) 1998 Sendmail, Inc.  All rights reserved.
 * Copyright (c) 1983, 1995-1997 Eric P. Allman.  All rights reserved.
 * Copyright (c) 1988, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the sendmail distribution.
 * 
 * A copy of the above mentioned LICENSE file can be found in 
 * LICENSE.setproctitle.
 * 
 * Ported for use with KDE by Waldo Bastian <bastian@kde.org>
 */

#ifndef _SETPROCTITLE_H_
#define _SETPROCTITLE_H_

#ifdef __hpux
# define SPT_TYPE       SPT_PSTAT
#endif

#ifdef _AIX3
# define SPT_PADCHAR    '\0'    /* pad process title with nulls */
#endif

#ifdef _AIX4
# define SPT_PADCHAR    '\0'    /* pad process title with nulls */
#endif

#ifdef AIX                      /* AIX/RT compiler pre-defines this */
# define SPT_PADCHAR    '\0'    /* pad process title with nulls */
#endif

#ifdef DGUX_5_4_2
# define DGUX           1
#endif

#ifdef  DGUX
# define SPT_TYPE       SPT_NONE        /* don't use setproctitle */
#endif

/*
**  Apple Rhapsody
**      Contributed by Wilfredo Sanchez <wsanchez@apple.com>
*/

#ifdef __APPLE__
# define SPT_TYPE       SPT_PSSTRINGS
# define SPT_PADCHAR    '\0'    /* pad process title with nulls */
#endif

#if defined(BSD4_4) && !defined(__bsdi__) && !defined(__GNU__)
# define SPT_TYPE       SPT_PSSTRINGS   /* use PS_STRINGS pointer */
#endif

#ifdef __bsdi__
# if defined(_BSDI_VERSION) && _BSDI_VERSION >= 199312
/* version 1.1 or later */
#  undef SPT_TYPE
#  define SPT_TYPE      SPT_BUILTIN     /* setproctitle is in libc */
# else
/* version 1.0 or earlier */
#  define SPT_PADCHAR   '\0'    /* pad process title with nulls */
# endif
#endif

#if defined(__QNX__)
# define SPT_TYPE       SPT_REUSEARGV
# define SPT_PADCHAR    '\0'    /* pad process title with nulls */
#endif

#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__)
# if defined(__NetBSD__) || defined(__DragonFly__)
#  undef SPT_TYPE
#  define SPT_TYPE      SPT_BUILTIN     /* setproctitle is in libc */
# endif
# if defined(__FreeBSD__)
#  undef SPT_TYPE
#  if __FreeBSD__ >= 2
#   include <osreldate.h>
#   if __FreeBSD_version >= 199512      /* 2.2-current when it appeared */
#    include <sys/types.h>
#    include <libutil.h>
#    define SPT_TYPE    SPT_BUILTIN
#   endif
#  endif
#  ifndef SPT_TYPE
#   define SPT_TYPE     SPT_REUSEARGV
#   define SPT_PADCHAR  '\0'            /* pad process title with nulls */
#  endif
# endif
# if defined(__OpenBSD__)
#  undef SPT_TYPE
#  define SPT_TYPE      SPT_BUILTIN     /* setproctitle is in libc */
# endif
#endif

#ifdef __GNU_HURD__
# define SPT_TYPE       SPT_CHANGEARGV
#endif

/* SCO UNIX 3.2v4.2/Open Desktop 3.0 */
#ifdef _SCO_unix_4_2
# define _SCO_unix_
#endif

/* SCO UNIX 3.2v4.0 Open Desktop 2.0 and earlier */
#ifdef _SCO_unix_
# define SPT_TYPE       SPT_SCO         /* write kernel u. area */
#endif


#ifdef __linux__
# define SPT_PADCHAR    '\0'            /* pad process title with nulls */
#endif

#ifdef _SEQUENT_
# define SPT_TYPE       SPT_NONE        /* don't use setproctitle */
#endif

#ifdef apollo
# define SPT_TYPE       SPT_NONE        /* don't use setproctitle */
#endif

#ifdef __svr5__
# define SPT_PADCHAR            '\0'    /* pad process title with nulls */
#endif
 
#ifdef NCR_MP_RAS2
# define SPT_TYPE  SPT_NONE
#endif

#ifdef NCR_MP_RAS3
# define SPT_TYPE       SPT_NONE
#endif
 
#ifdef sony_news
# ifndef __svr4
#  ifndef SPT_TYPE
#   define SPT_TYPE     SPT_SYSMIPS     /* use sysmips() (OS 6.0.2 or later) */
#  endif
# endif
#endif


extern void kdeinit_initsetproctitle(int, char **, char **);
extern void kdeinit_setproctitle(const char *, ...)
#ifdef __GNUC__
    __attribute__ (( format ( printf, 1, 2 ) ) )
#endif
;

#endif


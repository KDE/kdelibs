/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 2007 The ProFTPD Project team           //krazy:exclude=copyright
 * Copyright (c) 2007 Alex Merry <alex.merry@kdemail.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#include "proctitle.h"
#include <config.h>
#include <config-kdeinit.h>

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#define PF_ARGV_NONE      0
#define PF_ARGV_NEW       1
#define PF_ARGV_WRITEABLE 2
#define PF_ARGV_PSTAT     3
#define PF_ARGV_PSSTRINGS 4

#ifdef HAVE_SETPROCTITLE
#  define PF_ARGV_TYPE PF_ARGV_NONE
#  ifdef HAVE_SYS_TYPES_H
#    include <sys/types.h>
#  endif /* HAVE_SYS_TYPES_H */
#  ifdef HAVE_UNISTD_H
#    include <unistd.h>
#  endif /* HAVE_UNISTD_H */

#else /* HAVE_SETPROCTITLE */
#  ifdef __GNU_HURD__
#    define PF_ARGV_TYPE PF_ARGV_NEW
#  else /* __GNU_HURD__ */
#    define PF_ARGV_TYPE PF_ARGV_WRITEABLE

#    if defined(HAVE_SYS_PSTAT_H) && defined(HAVE_PSTAT)
#      include <sys/pstat.h>
#      undef PF_ARGV_TYPE
#      define PF_ARGV_TYPE PF_ARGV_PSTAT
#    endif /* HAVE_SYS_PSTAT_H && HAVE_PSTAT */

#    ifdef HAVE_SYS_EXEC_H
#      include <sys/exec.h>
#      ifdef PS_STRINGS
#        include <machine/vmparam.h>
#        undef PF_ARGV_TYPE
#        define PF_ARGV_TYPE PF_ARGV_PSSTRINGS
#      endif /* PS_STRINGS */
#    endif /* HAVE_SYS_EXEC_H */

#  endif /* !__GNU_HURD__ */

#endif /* !HAVE_SETPROCTITLE */

#ifdef HAVE___PROGNAME
extern char *__progname;
#endif /* HAVE___PROGNAME */
#ifdef HAVE___PROGNAME_FULL
extern char *__progname_full;
#endif /* HAVE___PROGNAME_FULL */
extern char **environ;

static char **Argv = NULL;

#if PF_ARGV_TYPE == PF_ARGV_WRITEABLE   /* Only this mode uses LastArgv */
static char *LastArgv = NULL;
static char *cleanUpTo = NULL;
#endif

/**
 * Set up the memory space for setting the proctitle
 */
void proctitle_init(int argc, char *argv[], char *envp[]) {
    register int i, envpsize;
    char **p;

    /* Move the environment so proctitle_set can use the space. */
    for ( i = envpsize = 0; envp[i] != NULL; i++ ) {
        envpsize += strlen(envp[i]) + 1;
    }

    if ((p = (char **) malloc((i + 1) * sizeof(char *))) != NULL) {
        environ = p;

        for (i = 0; envp[i] != NULL; i++) {
            if ((environ[i] = static_cast<char *>(malloc(strlen(envp[i]) + 1))) != NULL) {
                strcpy(environ[i], envp[i]);
            }
        }

        environ[i] = NULL;
    }

    Argv = argv;

# if PF_ARGV_TYPE == PF_ARGV_WRITEABLE   /* Only this mode uses LastArgv */
    for (i = 0; i < argc; i++) {
        if (!i || (LastArgv + 1 == argv[i])) {
            LastArgv = argv[i] + strlen(argv[i]);
        }
    }
    cleanUpTo = LastArgv;

    for (i = 0; envp[i] != NULL; i++) {
        /* must not overwrite XDG_SESSION_COOKIE */
        if (!strncmp(envp[i], "XDG_", 4))
            break;
        if ((LastArgv + 1) == envp[i]) {
            LastArgv = envp[i] + strlen(envp[i]);
        }
    }
#endif

# ifdef HAVE___PROGNAME
    /* Set the __progname variable so glibc and company
     * don't go nuts.
     */
    __progname = strdup("kdeinit4");
# endif /* HAVE___PROGNAME */
# ifdef HAVE___PROGNAME_FULL
    /* __progname_full too */
    __progname_full = strdup(argv[0]);
# endif /* HAVE___PROGNAME_FULL */
}

void proctitle_set(const char *fmt, ...) {
    va_list msg;
    static char statbuf[BUFSIZ];

#ifndef HAVE_SETPROCTITLE
# if PF_ARGV_TYPE == PF_ARGV_PSTAT
    union pstun pst;
# endif /* PF_ARGV_PSTAT */
    char *p;
    int i;
#endif /* HAVE_SETPROCTITLE */

    if ( !fmt ) {
        return;
    }

    va_start(msg, fmt);

    memset(statbuf, 0, sizeof(statbuf));

#ifdef HAVE_SETPROCTITLE
# if __FreeBSD__ >= 4 && !defined(FREEBSD4_0) && !defined(FREEBSD4_1)
    /* FreeBSD's setproctitle() automatically prepends the process name. */
    vsnprintf(statbuf, sizeof(statbuf), fmt, msg);

# else /* FREEBSD4 */
    /* Manually append the process name for non-FreeBSD platforms. */
    snprintf(statbuf, sizeof(statbuf), "%s", "kdeinit4: ");
    vsnprintf(statbuf + strlen(statbuf),
              sizeof(statbuf) - strlen(statbuf),
              fmt,
              msg);

# endif /* FREEBSD4 */
    setproctitle("%s", statbuf);

#else /* HAVE_SETPROCTITLE */
    /* Manually append the process name for non-setproctitle() platforms. */
    snprintf(statbuf, sizeof(statbuf), "%s", "kdeinit4: ");
    vsnprintf(statbuf + strlen(statbuf),
              sizeof(statbuf) - strlen(statbuf),
              fmt,
              msg);

#endif /* HAVE_SETPROCTITLE */

    va_end(msg);

#ifdef HAVE_SETPROCTITLE
    return;
#else
    i = strlen(statbuf);

# if PF_ARGV_TYPE == PF_ARGV_NEW
    /* We can just replace argv[] arguments.  Nice and easy. */
    Argv[0] = statbuf;
    Argv[1] = NULL;
# endif /* PF_ARGV_NEW */

# if PF_ARGV_TYPE == PF_ARGV_WRITEABLE
    const int maxlen = (LastArgv - Argv[0]) - 1;
    /* We can overwrite individual argv[] arguments.  Semi-nice. */
    snprintf(Argv[0], maxlen, "%s", statbuf);
    p = &Argv[0][i];
    /* Clear the rest used by arguments, but don't clear the memory
       that is usually used for environment variables. Some
       tools, like ConsoleKit must have access to the process'es initial
       environment (more exact, the XDG_SESSION_COOKIE variable stored there).
       If this code causes another side effect, we have to specifically
       always append those variables to our environment. */
    while (p < cleanUpTo)
        *p++ = '\0';

    Argv[1] = NULL;
# endif /* PF_ARGV_WRITEABLE */

# if PF_ARGV_TYPE == PF_ARGV_PSTAT
    pst.pst_command = statbuf;
    pstat(PSTAT_SETCMD, pst, i, 0, 0);
# endif /* PF_ARGV_PSTAT */

# if PF_ARGV_TYPE == PF_ARGV_PSSTRINGS
    PS_STRINGS->ps_nargvstr = 1;
    PS_STRINGS->ps_argvstr = statbuf;
# endif /* PF_ARGV_PSSTRINGS */

#endif /* HAVE_SETPROCTITLE */
}

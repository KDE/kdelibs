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

#include "setproctitle.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE
#include <stdio.h>
#include <stdarg.h>                                                      
#include <sys/ioctl.h>
#include <sys/param.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* _PATH_KMEM should be defined in <paths.h> */
#ifndef _PATH_KMEM
# define _PATH_KMEM	"/dev/kmem"
#endif

#define SPACELEFT(buf, ptr)	(sizeof buf - ((ptr) - buf))


/*
**  SETPROCTITLE -- set process title for ps
**
**	Parameters:
**		fmt -- a printf style format string.
**		a, b, c -- possible parameters to fmt.
**
**	Returns:
**		none.
**
**	Side Effects:
**		Clobbers argv of our main procedure so ps(1) will
**		display the title.
*/

#define SPT_NONE	0	/* don't use it at all */
#define SPT_REUSEARGV	1	/* cover argv with title information */
#define SPT_BUILTIN	2	/* use libc builtin */
#define SPT_PSTAT	3	/* use pstat(PSTAT_SETCMD, ...) */
#define SPT_PSSTRINGS	4	/* use PS_STRINGS->... */
#define SPT_SYSMIPS	5	/* use sysmips() supported by NEWS-OS 6 */
#define SPT_SCO		6	/* write kernel u. area */
#define SPT_CHANGEARGV	7	/* write our own strings into argv[] */

#ifndef SPT_TYPE
# define SPT_TYPE	SPT_REUSEARGV
#endif

#if SPT_TYPE != SPT_NONE && SPT_TYPE != SPT_BUILTIN

# if SPT_TYPE == SPT_PSTAT
#  include <sys/pstat.h>
# endif
# if SPT_TYPE == SPT_PSSTRINGS
#  include <machine/vmparam.h>
#  include <sys/exec.h>
#  ifndef PS_STRINGS	/* hmmmm....  apparently not available after all */
#   undef SPT_TYPE
#   define SPT_TYPE	SPT_REUSEARGV
#  else
#   ifndef NKPDE			/* FreeBSD 2.0 */
#    define NKPDE 63
typedef unsigned int	*pt_entry_t;
#   endif
#  endif
# endif

# if SPT_TYPE == SPT_PSSTRINGS || SPT_TYPE == SPT_CHANGEARGV
#  define SETPROC_STATIC	static
# else
#  define SETPROC_STATIC
# endif

# if SPT_TYPE == SPT_SYSMIPS
#  include <sys/sysmips.h>
#  include <sys/sysnews.h>
# endif

# if SPT_TYPE == SPT_SCO
#  include <sys/immu.h>
#  include <sys/dir.h>
#  include <sys/user.h>
#  include <sys/fs/s5param.h>
#  if PSARGSZ > MAXLINE
#   define SPT_BUFSIZE	PSARGSZ
#  endif
# endif

# ifndef SPT_PADCHAR
#  ifdef _AIX
#   define SPT_PADCHAR	'\0'
#  else
#   define SPT_PADCHAR	' '
#  endif
# endif

#endif /* SPT_TYPE != SPT_NONE && SPT_TYPE != SPT_BUILTIN */

# ifndef SPT_BUFSIZE
#  define SPT_BUFSIZE	2048
# endif

/*
**  Pointers for setproctitle.
**	This allows "ps" listings to give more useful information.
*/

char		**Argv = NULL;		/* pointer to argument vector */
char		*LastArgv = NULL;	/* end of argv */

void
kdeinit_initsetproctitle(int argc, char **argv, char **envp)
{
	register int i, envpsize = 0;
	extern char **environ;

	/*
	**  Move the environment so setproctitle can use the space at
	**  the top of memory.
	*/

	for (i = 0; envp[i] != NULL; i++)
		envpsize += strlen(envp[i]) + 1;
	environ = (char **) malloc(sizeof (char *) * (i + 1));
    if (environ == NULL)
        return;

	for (i = 0; envp[i] != NULL; i++)
    {
	environ[i] = strdup(envp[i]);
    }
	environ[i] = NULL;

	/*
	**  Save start and extent of argv for setproctitle.
	*/

	Argv = argv;

	/*
	**  Determine how much space we can use for setproctitle.  
	**  Use all contiguous argv and envp pointers starting at argv[0]
 	*/
	for (i = 0; i < argc; i++)
	{
		if (i==0 || LastArgv + 1 == argv[i])
			LastArgv = argv[i] + strlen(argv[i]);
		else
			continue;
	}
	for (i=0; envp[i] != NULL; i++)
	{
		if (LastArgv + 1 == envp[i])
			LastArgv = envp[i] + strlen(envp[i]);
		else
			continue;
	}
}

#if SPT_TYPE != SPT_BUILTIN

/*VARARGS1*/
static void
setproctitle(const char *fmt, ...)
{
# if SPT_TYPE != SPT_NONE
	register char *p;
	register int i;
	SETPROC_STATIC char buf[SPT_BUFSIZE];
	va_list ap;
#  if SPT_TYPE == SPT_PSTAT
	union pstun pst;
#  endif
#  if SPT_TYPE == SPT_SCO
	off_t seek_off;
	static int kmem = -1;
#warning (rikkus) kmempid is declared as int, should be long
	static int kmempid = -1;
	struct user u;
#  endif

	p = buf;

	/* print kdeinit: heading for grep */
	(void) strcpy(p, "kdeinit: ");
	p += strlen(p);

	/* print the argument string */
	va_start(ap, fmt);
	(void) vsnprintf(p, SPACELEFT(buf, p), fmt, ap);
	va_end(ap);

	i = strlen(buf);

#  if SPT_TYPE == SPT_PSTAT
	pst.pst_command = buf;
	pstat(PSTAT_SETCMD, pst, i, 0, 0);
#  endif
#  if SPT_TYPE == SPT_PSSTRINGS
	PS_STRINGS->ps_nargvstr = 1;
	PS_STRINGS->ps_argvstr = buf;
#  endif
#  if SPT_TYPE == SPT_SYSMIPS
	sysmips(SONY_SYSNEWS, NEWS_SETPSARGS, buf);
#  endif
#  if SPT_TYPE == SPT_SCO
	if (kmem < 0 || kmempid != getpid())
	{
		if (kmem >= 0)
			close(kmem);
		kmem = open(_PATH_KMEM, O_RDWR, 0);
		if (kmem < 0)
			return;
		(void) fcntl(kmem, F_SETFD, 1);
		kmempid = getpid();
	}
	buf[PSARGSZ - 1] = '\0';
	seek_off = UVUBLK + (off_t) u.u_psargs - (off_t) &u;
	if (lseek(kmem, (off_t) seek_off, SEEK_SET) == seek_off)
		(void) write(kmem, buf, PSARGSZ);
#  endif
#  if SPT_TYPE == SPT_REUSEARGV
	if (i > LastArgv - Argv[0] - 2)
	{
		i = LastArgv - Argv[0] - 2;
		buf[i] = '\0';
	}
	(void) strcpy(Argv[0], buf);
	p = &Argv[0][i];
	while (p < LastArgv)
		*p++ = SPT_PADCHAR;
	Argv[1] = NULL;
#  endif
#  if SPT_TYPE == SPT_CHANGEARGV
	Argv[0] = buf;
	Argv[1] = 0;
#  endif
# endif /* SPT_TYPE != SPT_NONE */
}

#endif /* SPT_TYPE != SPT_BUILTIN */
/*
**  SM_SETPROCTITLE -- set process task and set process title for ps
**
**	Possibly set process status and call setproctitle() to
**	change the ps display.
**
**	Parameters:
**		status -- whether or not to store as process status
**		fmt -- a printf style format string.
**		a, b, c -- possible parameters to fmt.
**
**	Returns:
**		none.
*/

/*VARARGS2*/
void
kdeinit_setproctitle(const char *fmt, ...)
{
	char buf[SPT_BUFSIZE];

	va_list ap;
	/* print the argument string */
	va_start(ap, fmt);
	(void) vsnprintf(buf, SPT_BUFSIZE, fmt, ap);
	va_end(ap);

	setproctitle("%s", buf);
}


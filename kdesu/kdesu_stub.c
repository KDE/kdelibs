/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
 *
 * kdesu_stub.c: KDE su executes this stub through su or ssh. This stub in turn
 *               executes the target program. Before that, startup parameters
 *               are sent through stdin.
 *
 *
 * Available parameters:
 *
 *   Parameter       Description         Format (csl = comma separated list)
 *
 * - kdesu_stub      Header              "ok" | "stop"
 * - display         X11 display         string
 * - display_auth    X11 authentication  "type cookie" pair
 * - dcopserver      KDE dcopserver      csl of netids
 * - dcop_auth       DCOP authentication csl of "type cookie" pairs for DCOP
 * - ice_auth        ICE authentication  csl of "type cookie" pairs for ICE
 * - command         Command to run      string
 * - path            PATH env. var       string
 * - build_sycoca    Rebuild sycoca?     "yes" | "no"
 * - user            Target user         string
 * - priority        Process priority    0 <= int <= 100
 * - scheduler       Process scheduler   "fifo" | "normal"
 * - kappstart_pid   PID the app uses    int
 *                   for _NET_WM_PID
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pwd.h>
#include <termios.h>
#include <signal.h>

#ifdef HAVE_INITGROUPS
#include <grp.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>

#ifdef POSIX1B_SCHEDULING
#include <sched.h>
#endif

/**
 * Params sent by the peer.
 */

struct param_struct 
{
    const char *name;
    char *value;
};

struct param_struct params[] = 
{
    { "kdesu_stub", 0L },
    { "display", 0L },
    { "display_auth", 0L },
    { "dcopserver", 0L },
    { "dcop_auth", 0L },
    { "ice_auth", 0L },
    { "command", 0L },
    { "path", 0L },
    { "xwindows_only", 0L },
    { "user", 0L },
    { "priority", 0L },
    { "scheduler", 0L },
    { "app_start_pid", 0L }
};

#define P_HEADER 0
#define P_DISPLAY 1
#define P_DISPLAY_AUTH 2
#define P_DCOPSERVER 3
#define P_DCOP_AUTH 4
#define P_ICE_AUTH 5
#define P_COMMAND 6
#define P_PATH 7
#define P_XWIN_ONLY 8
#define P_USER 9
#define P_PRIORITY 10
#define P_SCHEDULER 11
#define P_APP_START_PID 12
#define P_LAST 13

/* Prototypes */
char *xmalloc(size_t);
char *xrealloc(char *ptr, int size);
int xsetenv(const char *name, const char *value);
char *xstrdup(char *src);
char **xstrsep(char *str);

/**
 * Safe malloc functions.
 */
char *xmalloc(size_t size)
{
    char *ptr = malloc(size);
    if (ptr) return ptr;
    perror("malloc()");
    exit(1);
}


char *xrealloc(char *ptr, int size)
{
    ptr = realloc(ptr, size);
    if (ptr) return ptr;
    perror("realloc()");
    exit(1);
}


/**
 * Solaris does not have a setenv()...
 */
int xsetenv(const char *name, const char *value)
{
    char *s = malloc(strlen(name)+strlen(value)+2);
    if (!s) return -1;
    strcpy(s, name);
    strcat(s, "=");
    strcat(s, value);
    return putenv(s); /* yes: no free()! */
}

/**
 * Safe strdup and strip newline
 */
char *xstrdup(char *src)
{
    int len = strlen(src);
    char *dst = xmalloc(len+1);
    strcpy(dst, src);
    if (dst[len-1] == '\n')
	dst[len-1] = '\000';
    return dst;
}

/**
 * Split comma separated list.
 */
char **xstrsep(char *str)
{
    int i = 0, size = 10;
    char **list = (char **) xmalloc(size * sizeof(char *));
    char *ptr = str, *nptr;
    while ((nptr = strchr(ptr, ',')) != 0L) 
    {
	if (i > size-2)
	    list = realloc(list, (size *= 2) * sizeof(char *));
	*nptr = '\000';
	list[i++] = ptr;
	ptr = nptr+1;
    }
    if (*ptr != '\000')
	list[i++] = ptr;
    list[i] = 0L;
    return list;
}

#define BUFSIZE	8192

/**
 * The main program
 */

int main()
{
    char buf[BUFSIZE+1];
#ifndef QWS
    char xauthority[200];
#endif
    char iceauthority[200];
    char **host, **auth;
    int i/*, res, sycoca*/, prio;
    pid_t pid;
    FILE *fout;
    struct passwd *pw;

    /* Get startup parameters. */

    for (i=0; i<P_LAST; i++) 
    {
	printf("%s\n", params[i].name);
	fflush(stdout);
	if (fgets(buf, BUFSIZE, stdin) == 0L) 
	{
	    printf("end\n"); fflush(stdout);
	    perror("kdesu_stub: fgets()");
	    exit(1);
	}
	params[i].value = xstrdup(buf);
	/* Installation check? */
	if ((i == 0) && !strcmp(params[i].value, "stop")) 
	{
	    printf("end\n");
	    exit(0);
	}
    }
    printf("end\n");
    fflush(stdout);

fprintf(stderr, "PATH :%s\n", params[P_PATH].value);
    xsetenv("PATH", params[P_PATH].value);
    xsetenv("KDE_APP_START_PID", params[P_APP_START_PID].value);

    /* Do we need to change uid? */

    pw = getpwnam(params[P_USER].value);
    if (pw == 0L) 
    {
	printf("kdesu_stub: user %s does not exist!\n", params[P_USER].value);
	exit(1);
    }
    xsetenv("HOME", pw->pw_dir);
    sprintf(buf, "%s/.kde/", pw->pw_dir);
    xsetenv("KDEHOME", buf);

    /* Set scheduling/priority */

    prio = atoi(params[P_PRIORITY].value);
    if (!strcmp(params[P_SCHEDULER].value, "realtime")) 
    {
#ifdef POSIX1B_SCHEDULING
	struct sched_param sched;
	int min = sched_get_priority_min(SCHED_FIFO);
	int max = sched_get_priority_max(SCHED_FIFO);
	sched.sched_priority = min + (int) (((double) prio) * (max - min) / 100 + 0.5);
	sched_setscheduler(0, SCHED_FIFO, &sched);
#else
	printf("kdesu_stub: realtime scheduling not supported\n");
#endif
    } else 
    {
	int val = 20 - (int) (((double) prio) * 40 / 100 + 0.5);
	setpriority(PRIO_PROCESS, getpid(), val);
    }

    /* Drop privileges (this is permanent) */

    if (getuid() != pw->pw_uid) 
    {
	if (setgid(pw->pw_gid) == -1) 
	{
	    perror("kdesu_stub: setgid()");
	    exit(1);
	}
#ifdef HAVE_INITGROUPS
	if (initgroups(pw->pw_name, pw->pw_gid) == -1) 
	{
	    perror("kdesu_stub: initgroups()");
	    exit(1);
	}
#endif
	if (setuid(pw->pw_uid) == -1) 
	{
	    perror("kdesu_stub: setuid()");
	    exit(1);
	}
	xsetenv("HOME", pw->pw_dir);
    }

    /* Handle display */

    if (strcmp(params[P_DISPLAY].value, "no")) 
    {
#ifndef QWS
	xsetenv("DISPLAY", params[P_DISPLAY].value);
	if (params[P_DISPLAY_AUTH].value[0]) 
	{
           int	fd2;
           /*
           ** save umask and set to 077, so we create those files only
	   ** readable for root. (if someone else could read them, we
	   ** are in deep shit).
           */
	   int oldumask = umask(077);

           strcpy(xauthority, "/tmp/xauth.XXXXXXXXXX");
	   fd2 = mkstemp(xauthority);
	   umask(oldumask);

           if (fd2 == -1) {
                perror("kdesu_stub: mkstemp()");
                exit(1);
           } else
                close(fd2);
           xsetenv("XAUTHORITY", xauthority);

	   fout = popen("xauth >/dev/null 2>&1","w");
           if (fout == NULL)
	   {
		perror("kdesu_stub: popen(xauth)");
		exit(1);
	   }
	   fprintf(fout, "add %s %s\n", params[P_DISPLAY].value,
		    params[P_DISPLAY_AUTH].value);
	   pclose(fout);
	}
#else
	xsetenv("DISPLAY", params[P_DISPLAY].value);
#endif
    }


    /* Handle DCOP */

    if (strcmp(params[P_DCOPSERVER].value, "no")) 
    {
	xsetenv("DCOPSERVER", params[P_DCOPSERVER].value);
	host = xstrsep(params[P_DCOPSERVER].value);
	auth = xstrsep(params[P_ICE_AUTH].value);
	if (host[0]) 
	{
            int fd;
	    int	oldumask = umask(077);

            strcpy(iceauthority, "/tmp/iceauth.XXXXXXXXXX");
            fd = mkstemp(iceauthority);
	    umask(oldumask);
            if (fd == -1) {
                perror("kdesu_stub: mkstemp()");
                exit(1);
            } else
                close(fd);
	    xsetenv("ICEAUTHORITY", iceauthority);

	    fout = popen("iceauth >/dev/null 2>&1", "w");
	    if (!fout) {
		perror("kdesu_stub: popen iceauth");
		exit(1);
	    }
	    for (i=0; host[i]; i++)
		fprintf(fout, "add ICE \"\" %s %s\n", host[i], auth[i]);
	    auth = xstrsep(params[P_DCOP_AUTH].value);
	    for (i=0; host[i]; i++)
		fprintf(fout, "add DCOP \"\" %s %s\n", host[i], auth[i]);
	    pclose(fout);
	}
    }

    /* Don't do session management with kdesu apps. I think this is the
     * best thing to do. I would be potentially insecure, it is unclear to
     * me how the app can ever know its restart command and it would require 
     * the user to enter passwords at startup of a session. */

     xsetenv("SESSION_MANAGER", "none");

    /* Rebuild the sycoca and start kdeinit? */

    if (strcmp(params[P_XWIN_ONLY].value, "no"))
    {
	system("kdeinit --suicide");
    }

fprintf(stderr," before forking\n");

    /* Execute the command */

    pid = fork();
    if (pid == -1) 
    {
	perror("kdesu_stub: fork()");
	exit(1);
    }
    if (pid) 
    {
	/* Parent: wait for child, delete tempfiles and return. */
	int ret, state, xit = 1;
	while (1) 
	{
	    ret = waitpid(pid, &state, 0);
	    if (ret == -1) 
	    {
		if (errno == EINTR)
		    continue;
		if (errno != ECHILD)
		    perror("kdesu_stub: waitpid()");
		break;
	    }
	    if (WIFEXITED(state))
		xit = WEXITSTATUS(state);
	}

#ifndef QWS
	unlink(xauthority);
#endif
	unlink(iceauthority);
	exit(xit);
    } else 
    {
	/* Child: exec command. */
	setsid();
	sprintf(buf, "%s", params[P_COMMAND].value);
	execl("/bin/sh", "sh", "-c", buf, 0L);
	perror("kdesu_stub: exec()");
	_exit(1);
    }
}

#include <stdio.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>

#undef ARTSWRAPPER_DEBUG

/*
 * adjust_priority
 *
 * sets realtime priority
 */

#include "config.h"

void rt_error()
{
  	fprintf(stderr," This means that this program will likely not be able\n");
  	fprintf(stderr," to produce acceptable sound (without clicks and breaks).");
	fprintf(stderr,"\n\n");
}

#ifdef HAVE_REALTIME_SCHED
#include <sched.h>

void adjust_priority()
{
	int sched = sched_getscheduler(0);
	if(sched == SCHED_FIFO || sched == SCHED_RR)
	{
		printf(">> since the scheduling policy is not standard, I assume\n");
		printf("   it has been adjusted to fit the needs of realtime audio\n");
	}
	else
	{
		struct sched_param sp;
		long priority = (sched_get_priority_max(SCHED_FIFO) +
			             sched_get_priority_min(SCHED_FIFO))/2;
		              
		sp.sched_priority = priority;

		if(sched_setscheduler(0, SCHED_FIFO, &sp) == 0)
		{
			printf(">> running as realtime process now (priority %ld)\n",
																	priority);
		}
		else
		{
			fprintf(stderr,"\nWARNING: Can't get realtime priority ");
			fprintf(stderr," (try running as root)!\n");
			rt_error();
		}
	}
}
#else
void adjust_priority()
{
	int prio;

	prio = getpriority(PRIO_PROCESS,getpid());
	if(prio < -10)
	{
		setpriority(PRIO_PROCESS,getpid(),-17);
		prio = getpriority(PRIO_PROCESS,getpid());
	}

  	fprintf(stderr,
		"\nWARNING: Your system doesn't support realtime scheduling.\n");
	rt_error();

	if(prio > -10) {
		printf(">> synthesizer priority is %d (which is unacceptable,",prio);
		printf(" try running as root)\n");
	}
	else {
		printf(">> synthesizer priority is %d (which is the best\n",prio);
		printf("   we can get out of a non realtime system)\n");
	}
}
#endif

/*
 * check_security
 *
 * checks that the path (file or directory) is owned by root, which makes
 * it save to assume that nobody can temper with the files in that directory,
 * and so we can execute the program with realtime priority
 *
 * result = 0  => not secure
 * result = 1  => secure (well - reasonable secure - if root installs
 *   directories as world writeable for instance, still everybody can
 *   do everything)
 */

int check_security(const char *path)
{
	struct stat stat_buffer;

#ifdef ARTSWRAPPER_DEBUG
	printf("Checking security for %s\n",path);
#endif
	/*
	 * No, we can't execute things in paths with zero length (for instance
	 * current directory) savely. If path has length 1, it must be /, because
	 * otherwise we will be executing stuff in a one
	 */

	if(strlen(path) == 0) return 0;
	if(strlen(path) == 1 && path[0] != '/') return 0;

	if(strlen(path) > 1)
	{
		char *basedir = strdup(path);
		int end = 0;
		int basedir_security;
		int i;

		for(i=0;i<strlen(basedir);i++)
		{
			if(basedir[i] == '/') end = i;
		}

		if(end == 0 && basedir[0] == '/') end = 1; /* root directory */
		basedir[end] = 0;

		basedir_security = check_security(basedir);
		free(basedir);

		if(basedir_security == 0) return 0;
	}

	stat(path, &stat_buffer); 

	if(stat_buffer.st_uid != 0)
	{
		fprintf(stderr,"artswrapper: can't use suid, because security for\n");
		fprintf(stderr,"    <%s> is critical, since it isn't owned by root\n",
				path);
		return 0;
	}

#ifdef ARTSWRAPPER_DEBUG
	printf("%s passed security check\n",path);
#endif
	/*
	 * more tests here, like "not world writeable" or "only writeable by root"?
	 */
	return 1;
}

int main(int argc, char **argv)
{
	const char *execute = EXECUTE;
	int secure;

	secure = check_security(execute);

	if(argc == 2)
	{
		if(strcmp(argv[1],"check") == 0)
		{
			if(secure)
			{
				printf("okay\n");
				exit(0);
			}
			else
			{
				printf("fail\n");
				exit(1);
			}
		}
	}
	if(!secure)
	{
		fprintf(stderr,"artswrapper: security check failed, leaving.\n");
		exit(255);
	}

	adjust_priority();

	/* drop root priviliges if running setuid root
	   (due to realtime priority stuff) */
	if (geteuid() != getuid()) setreuid(-1, getuid());

	execv(execute,argv);
	return 0;
}

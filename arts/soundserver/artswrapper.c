#include <stdio.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>

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

int main(int argc, char **argv)
{
	if(argc == 2)
	{
		if(strcmp(argv[1],"check") == 0)
		{
			/* backward compatibility with old artswrapper */
			printf("okay\n");
			return 0;
		}
	}

	adjust_priority();

	/* drop root priviliges if running setuid root
	   (due to realtime priority stuff) */
	if (geteuid() != getuid()) setreuid(-1, getuid());

	if(argc == 0)
		return 1;

	argv[0] = EXECUTE;
	execv(EXECUTE,argv);
	perror(EXECUTE);
	return 1;
}

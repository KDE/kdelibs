#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

void commandFromPid(int pid, char *name)
{
	char	buf[256], c;
	FILE	*f;
	int	i;

	name[0] = 0;
	snprintf(buf, 256, "/proc/%d/stat", pid);
	f = fopen(buf, "r");
	if (f == NULL)
		return;
	while ((c = fgetc(f)) != '(') ;
	i = 0;
	while ((c = fgetc(f)) != ')')
		name[i++] = c;
	name[i] = 0;
}

int findPid(const char *progname)
{
	char	name[256];
	DIR	*dir;
	int	pid = -1;
	struct dirent	*ds;

	dir = opendir("/proc");
	if (dir == NULL)
		return -1;
	while ((ds = readdir(dir)) !=NULL )
	{
		pid = -1;
		pid = atoi(ds->d_name);
		if (pid != -1)
		{
			commandFromPid(pid, name);
			if (strcmp(progname, name) == 0)
				return pid;
		}
	}

	return -1;
}

void usage()
{
	printf("usage: signal_proc [ -s signal_number ] [ -l ] -p <progname>\n");
}

int main(int argc, char **argv)
{
	int	pid = -1;
	char	progname[128] = {0};
	int	signal_number = -1, i, list_only = 0;

	for (i = 1; i < argc; i++)
	{
		if (argv[i][0] != '-')
		{
			usage();
			return(-1);
		}
		
		switch (argv[i][1])
		{
			case 'p':
				strncpy(progname, argv[++i], 128);
                                progname[127]='\0';
				break;
			case 's':
				signal_number = atoi(argv[++i]);
				break;
			case 'l':
				list_only = 1;
				break;
			default:
				usage();
				return -1;
		}
	}

	if (progname[0] == 0)
	{
		usage();
		return -1;
	}

	pid = findPid(progname);
	if (pid == -1)
	{
		fprintf(stderr, "no such program: %s\n", progname);
		return -1;
	}
	if (list_only)
	{
		fprintf(stdout, "PID: %d\n", pid);
		return 0;
	}

	if (signal_number != -1)
	{
		int	result;

		result = kill(pid, signal_number);
		if (result == -1)
		{
			if (errno == EPERM)
			{
				char	buf[256];

				fprintf(stderr, "operation not authorized, switching to root\n");
				snprintf(buf, 256, "kill -%d %d", signal_number, pid);
				if (execlp("kdesu", "kdesu", "-c", buf, NULL) == -1)
				{
					fprintf(stderr, "operation failed: %s\n", strerror(errno));
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "operation failed (invalid signal or no such process)\n");
				return -1;
			}
		}
	}
	else
	{
		fprintf(stderr, "only signal sending is currently supported\n");
		return -1;
	}
	return 0;
}

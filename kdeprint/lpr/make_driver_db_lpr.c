#include <config.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

#include "driverparse.h"

void simplifyModel(const char *modelname)
{
	char	*g;

	if ((g=strchr(modelname, ',')) != NULL)
		*g = 0;
	else if ((g=strchr(modelname, '(')) != NULL || (g=strchr(modelname, '{')) != NULL)
	{
		if (g != modelname)
		{
			if (isspace(*(g-1)))
				g--;
			*g = 0;
		}
	}
}

int parseApsFile(const char *filename, FILE *output)
{
	FILE	*apsfile;
	char	buf[256], modelname[256];
	char	*c, *d;

	apsfile = fopen(filename, "r");
	if (apsfile == NULL)
		return 0;
	while (fgets(buf, 255, apsfile) != NULL)
	{
		if ((c = strchr(buf, '\n')) != NULL)
			*c = 0;
		if (strlen(buf) == 0 || buf[0] == '#' || (c = strchr(buf, '|')) == NULL)
			continue;
		*c = 0;
		fprintf(output, "FILE=apsfilter/%s\n", c+1);
		d = strchr(buf, ' ');
		if (d)
		{
			*d = 0;
			strlcpy(modelname, d+1, 255);
			simplifyModel(modelname);
			fprintf(output, "MANUFACTURER=%s\n", buf);
			fprintf(output, "MODELNAME=%s\n", modelname);
			fprintf(output, "MODEL=%s\n", modelname);
			fprintf(output, "DESCRIPTION=%s %s (APSFilter + %s)\n", buf, d+1, c+1);
		}
		else
		{
			strlcpy(modelname, buf, 255);
			simplifyModel(modelname);
			fprintf(output, "MANUFACTURER=Unknown\n");
			fprintf(output, "MODELNAME=%s\n", modelname);
			fprintf(output, "MODEL=%s\n", modelname);
			fprintf(output, "DESCRIPTION=%s (APSFilter + %s)\n", buf, c+1);
		}
		fprintf(output, "\n");
	}
	fclose(apsfile);
	return 1;
}

void initAps(const char *base)
{
	char	drFile[256];
	DIR	*apsdir;
	struct dirent	*d;
	int	gsversion = 0;

	if (strstr(base, "apsfilter") == NULL)
		return;

	apsdir = opendir(base);
	if (apsdir == NULL)
		return;
	while ((d = readdir(apsdir)) != NULL)
	{
		if (strncmp(d->d_name, "printer-", 8) != 0)
			continue;
		if (isdigit(d->d_name[8]))
		{
			if (gsversion)
				continue;
			gsversion = 1;
		}
		snprintf(drFile, 256, "apsfilter:%s/%s", base, d->d_name);
		addFile(drFile);
	}
	closedir(apsdir);
}

char* nextWord(char *c)
{
	char	*d = c;
	while (*d && isspace(*d))
		d++;
	return d;
}

int parseIfhpFile(const char *filename, FILE *output)
{
	char	buf[1024];
	FILE	*in;
	char	model[32], desc[256];

	in = fopen(filename, "r");
	if (in == NULL)
		return 0;
	while (fgets(buf, 1023, in))
	{
		char	*c;
		if ((c = strchr(buf, '\n')) != NULL)
			*c = 0;
		c = buf;
		while (*c && isspace(*c))
			c++;
		if (*c == '#')
			continue;
		if (strncmp(c, "IfhpModel:", 10) == 0)
			strlcpy(model, nextWord(c+11), 31);
		else if (strncmp(c, "Description:", 12) == 0)
			strlcpy(desc, nextWord(c+13), 255);
		else if (strncmp(c, "EndEntry", 8) == 0)
		{
			char	*d = desc, *e, make[32] = {0};
			int	first_time = 1;
			do
			{
				e = strchr(d, ',');
				if (e)
					*e = 0;
				if (first_time)
				{
					char	*f = strchr(d, ' ');
					if (f)
						strlcpy(make, d, f-d);
					first_time = 0;
				}
				if (strstr(d, "Family") == NULL)
				{
					char	modelname[256] = {0};

					strlcpy(modelname, d, 255);
					simplifyModel(modelname);
					fprintf(output, "FILE=lprngtool/%s\n", model);
					fprintf(output, "MANUFACTURER=%s\n", make);
					fprintf(output, "MODEL=%s\n", modelname);
					fprintf(output, "MODELNAME=%s\n", modelname);
					fprintf(output, "DESCRIPTION=%s (IFHP + %s)\n", d, model);
					fprintf(output, "\n");
				}
				if (e)
				{
					d = e+1;
					while (*d && isspace(*d))
						d++;
				}
			} while (e);
		}
	}
	fclose(in);
	return 1;
}

void initIfhp(const char *base)
{
	char	path[256];

	snprintf(path, 255, "lprngtool:%s/printerdb", base);
	if (access(path+10, R_OK) == 0)
	{
		addFile(path);
	}
}

int main(int argc, char **argv)
{
	initFoomatic();
	registerHandler("apsfilter:", initAps, parseApsFile);
	registerHandler("lprngtool:", initIfhp, parseIfhpFile);
	return execute(argc, argv);
}

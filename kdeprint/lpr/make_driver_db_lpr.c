#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>

#define FOOMATIC_BASE	"/usr/share/foomatic/db/source"
#define APS_BASE		"/usr/share/apsfilter/setup"

char	**files;
int	nfiles, maxfiles;

void initFiles(void)
{
	maxfiles = 100;
	nfiles = 0;
	files = (char**)malloc(sizeof(char*) * maxfiles);
}

void freeFiles(void)
{
	int	i;
	for (i=0; i<nfiles; i++)
		free(files[i]);
	free(files);
}

void checkSize(void)
{
	if (nfiles == maxfiles)
	{
		maxfiles += 100;
		files = (char**)realloc(files, sizeof(char*) * maxfiles);
	}
}

void nextTag(FILE *f, char *tag, int len)
{
	int	p = 0;
	int	c;

	while (!feof(f) && fgetc(f) != '<') ;
	while (!feof(f) && p < (len-1) && (c = fgetc(f)) != '>')
		tag[p++] = c;
	tag[p] = 0;
}

void readValue(FILE *f, char *value, int len)
{
	char	c;
	int	p = 0;

	while (!feof(f) && p < (len-1) && (c = fgetc(f)) != '<')
		value[p++] = c;
	value[p] = 0;
}

int getMaticPrinterInfos(const char *id, char *make, char *model)
{
	char	filePath[256];
	FILE	*xmlFile;
	char	tag[32] = {0};
	int	n = 0;

	snprintf(filePath, 256, "%s/%s.xml", FOOMATIC_BASE, id);
	xmlFile = fopen(filePath, "r");
	if (xmlFile == NULL)
		return 0;
	while (!feof(xmlFile) && n < 2)
	{
		tag[0] = 0;
		nextTag(xmlFile, tag, 32);
		if (tag[0])
		{
			char	*c;

			if (strcmp(tag, "make") == 0)
				c = make;
			else if (strcmp(tag, "model") == 0)
				c = model;
			else
				continue;
			n++;
			readValue(xmlFile, c, 64);
		}
	}
	fclose(xmlFile);
	return 1;
}

int parseMaticDriverFile(char *driver, FILE *output)
{
	FILE	*drFile;
	char	name[32] = {0}, make[64] = {0}, model[64] = {0}, tag[32] = {0};
	char	id[128];

	drFile = fopen(driver, "r");
	if (drFile == NULL)
		return 0;
	while (!feof(drFile))
	{
		tag[0] = 0;
		nextTag(drFile, tag, 32);
		if (tag[0])
		{
			if (strcmp(tag, "name") == 0)
				readValue(drFile, name, 32);
			else if (strcmp(tag, "id") == 0)
			{
				readValue(drFile, id, 128);
				fprintf(output, "FILE=foomatic/%s/%s\n", id+8, name);
				make[0] = 0;
				model[0] = 0;
				getMaticPrinterInfos(id, make, model);
				fprintf(output, "MANUFACTURER=%s\n", make);
				fprintf(output, "MODELNAME=%s\n", model);
				fprintf(output, "MODEL=%s\n", model);
				fprintf(output, "DESCRIPTION=%s %s (Foomatic + %s)\n", make, model, name);
				fprintf(output, "\n");
			}
			else if (strcmp(tag, "/printers") == 0)
				break;
		}
	}
	fclose(drFile);
	return 1;
}

int initMatic(void)
{
	char	drPath[256];
	char	drFile[256];
	DIR	*foodir;
	struct dirent	*d;
	struct stat	st;
	int	n = 0;

	snprintf(drPath, 256, "%s/driver", FOOMATIC_BASE);
	foodir = opendir(drPath);
	if (foodir == NULL)
		return 0;
	while ((d = readdir(foodir)) != NULL)
	{
		snprintf(drFile, 256, "foomatic:%s/%s", drPath, d->d_name);
		if (stat(drFile+9, &st) != 0)
			continue;
		else if (!S_ISREG(st.st_mode))
			continue;
		checkSize();
		files[nfiles++] = strdup(drFile);
		n++;
	}
	closedir(foodir);
	return n;
}

void parseApsFile(char *filename, FILE *output)
{
	FILE	*apsfile;
	char	buf[256];
	char	*c, *d;

	apsfile = fopen(filename, "r");
	if (apsfile == NULL)
		return;
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
			fprintf(output, "MANUFACTURER=%s\n", buf);
			fprintf(output, "MODELNAME=%s\n", d+1);
			fprintf(output, "MODEL=%s\n", d+1);
			fprintf(output, "DESCRIPTION=%s %s (APSFilter + %s)\n", buf, d+1, c+1);
		}
		else
		{
			fprintf(output, "MANUFACTURER=Unknown\n");
			fprintf(output, "MODELNAME=%s\n", buf);
			fprintf(output, "MODEL=%s\n", buf);
			fprintf(output, "DESCRIPTION=%s (APSFilter + %s)\n", buf, c+1);
		}
		fprintf(output, "\n");
	}
	fclose(apsfile);
}

int initAps(void)
{
	char	drFile[256];
	DIR	*apsdir;
	struct dirent	*d;
	int	n = 0;

	apsdir = opendir(APS_BASE);
	if (apsdir == NULL)
		return 0;
	while ((d = readdir(apsdir)) != NULL)
	{
		if (strncmp(d->d_name, "printer-", 8) != 0)
			continue;
		snprintf(drFile, 256, "apsfilter:%s/%s", APS_BASE, d->d_name);
		checkSize();
		files[nfiles++] = strdup(drFile);
		n++;
	}
	closedir(apsdir);
	return n;
}

void parseAllFiles(FILE *output)
{
	int	i;
	for (i=0; i<nfiles; i++)
	{
		if (strncmp(files[i], "foomatic:", 9) == 0)
			parseMaticDriverFile(files[i]+9, output);
		else if (strncmp(files[i], "apsfilter:", 10) == 0)
			parseApsFile(files[i]+10, output);
		fprintf(stdout, "%d\n", i);
		fflush(stdout);
	}
}

int main(int argc, char **argv)
{
	FILE	*dbFile;
	int	n = 0;

	if (argc < 2 || argc > 3)
	{
		fprintf(stderr, "usage: make_driver_db <db_directory> [output_filename]\n");
		return -1;
	}
	if (argc == 3)
	{
		dbFile = fopen(argv[2], "w");
		if (dbFile == NULL)
		{
			fprintf(stderr, "unable to open DB file for writing\n");
			return -1;
		}
	}
	else
		dbFile = stdout;
	initFiles();
	n += initMatic();
	n += initAps();
	/* do it for other handlers */

	fprintf(stdout, "%d\n", n);
	fflush(stdout);
	parseAllFiles(dbFile);
	freeFiles();
	if (dbFile != stdout)
		fclose(dbFile);
	return 0;
}

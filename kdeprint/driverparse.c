#include "driverparse.h"

#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dlfcn.h>

char	**files = NULL;
int	nfiles = 0, maxfiles = 0;
int	nhandlers = 0, maxhandlers = 0;
int	nlibs = 0, maxlibs = 0;
typedef struct
{
	void (*init)(const char*);
	int (*parse)(const char*, FILE*);
	char	*name;
	int		namelen;
} handler;
handler	**handlers = NULL;
void	**libs = NULL;

void initHandlers(void)
{
	maxhandlers = 10;
	handlers = (handler**)malloc(sizeof(handler*) * maxhandlers);
}

void freeHandlers(void)
{
	int	i;
	for (i=0; i<nhandlers; i++)
	{
		free(handlers[i]->name);
		free(handlers[i]);
	}
	free(handlers);
}

void registerHandler(const char *name, void(*initf)(const char*), int(*parsef)(const char*, FILE*))
{
	handler	*h = (handler*)malloc(sizeof(handler));
	h->init = initf;
	h->parse = parsef;
	h->name = strdup(name);
	h->namelen = strlen(h->name);
	if (maxhandlers == 0)
		initHandlers();
	if (nhandlers == maxhandlers)
	{
		maxhandlers += 10;
		handlers = (handler**)realloc(handlers, sizeof(handler*) * maxhandlers);
	}
	handlers[nhandlers++] = h;
}

void addLib(const char *filename)
{
	void	*handle = dlopen(filename, RTLD_LAZY);
	if (handle)
	{
		void(*f)(void);
		if (nlibs == maxlibs)
		{
			maxlibs += 5;
			libs = (void**)realloc(libs, sizeof(void*) * maxlibs);
		}
		libs[nlibs++] = handle;
		f = dlsym(handle, "initialize");
		if (f)
		{
			(*f)();
		}
	}
}

void freeLibs(void)
{
	int	i;
	for (i=0; i<maxlibs; i++)
		dlclose(libs[i]);
	free(libs);
}

void initFiles(void)
{
	maxfiles = 100;
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

void addFile(const char *filename)
{
	if (maxfiles == 0)
		initFiles();
	checkSize();
	files[nfiles++] = strdup(filename);
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
	{
		if (isspace(c))
			c = ' ';
		value[p++] = c;
	}
	value[p] = 0;
}

void readComment(FILE *f, char *comment, int len)
{
	char	tag[32] = {0};

	do nextTag(f, tag, 32);
	while (tag[0] && strcmp(tag, "en") != 0 && strcmp(tag, "/comments") != 0);
	if (strcmp(tag, "en") == 0)
		readValue(f, comment, len);
}

int getMaticPrinterInfos(const char *base, const char *id, char *make, char *model, char *recomm, char *comment, char *pnpmake, char *pnpmodel)
{
	char	filePath[256];
	FILE	*xmlFile;
	char	tag[32] = {0};
	int	n = 0;
	int in_autodetect = 0;

	snprintf(filePath, 256, "%s/%s.xml", base, id);
	xmlFile = fopen(filePath, "r");
	if (xmlFile == NULL)
		return 0;
	while (!feof(xmlFile) && n < 6)
	{
		tag[0] = 0;
		nextTag(xmlFile, tag, 32);
		if (tag[0])
		{
			char	*c;
			
			if ( strcmp( tag, "autodetect" ) == 0 )
			{
				in_autodetect = 1;
				continue;
			}
			else if ( strcmp( tag, "/autodetect" ) == 0 )
			{
				in_autodetect = 0;
				continue;
			}
			else if (!make[0] && strcmp(tag, "make") == 0)
				c = make;
			else if (strcmp(tag, "model") == 0)
			{
				if ( in_autodetect && !pnpmodel[ 0 ] )
					c = pnpmodel;
				else if ( !in_autodetect && !model[ 0 ] )
					c = model;
				else
					continue;
			}
			else if ( !pnpmake[0] && in_autodetect && strcmp( tag, "manufacturer" ) == 0 )
				c = pnpmake;
			else if (!recomm[0] && strcmp(tag, "driver") == 0)
				c = recomm;
			else if (comment && !comment[0] && strcmp(tag, "comments") == 0)
			{
				readComment(xmlFile, comment, 4096);
				n++;
				continue;
			}
			else
				continue;
			n++;
			readValue(xmlFile, c, 64);
		}
	}
	fclose(xmlFile);
	return 1;
}

int parseMaticFile(const char *driver, FILE *output)
{
	FILE	*drFile;
	char	name[32] = {0},
			make[64] = {0},
			model[64] = {0},
			tag[32] = {0},
			recomm[64] = {0},
			comment[4096] = {0},
			comment2[4096] = {0},
			pnpmake[64] = {0},
			pnpmodel[64] = {0};
	char	id[128];
	char	path[256], *c;

	drFile = fopen(driver, "r");
	if (drFile == NULL)
		return 0;
	strncpy(path, driver, 255);
        path[ 255 ] = '\0';
	if ((c = strstr(path, "/driver/")) != NULL)
		*c = 0;
	c = comment;
	while (!feof(drFile))
	{
		tag[0] = 0;
		nextTag(drFile, tag, 32);
		if (tag[0])
		{
			if (strcmp(tag, "name") == 0)
				readValue(drFile, name, 32);
			else if (strcmp(tag, "comments") == 0)
				readComment(drFile, c, 4096);
			else if (strcmp(tag, "printers") == 0)
				c = comment2;
			else if (strcmp(tag, "printer") == 0)
			{
				id[0] = 0;
				comment2[0] = 0;
			}
			else if (strcmp(tag, "id") == 0)
				readValue(drFile, id, 128);
			else if (strcmp(tag, "/printer") == 0 && id[0])
			{
				fprintf(output, "FILE=foomatic/%s/%s\n", id+8, name);
				make[0] = 0;
				model[0] = 0;
				recomm[0] = 0;
				pnpmake[0] = 0;
				pnpmodel[0] = 0;
				getMaticPrinterInfos(path, id, make, model, recomm, NULL, pnpmake, pnpmodel);
				fprintf(output, "MANUFACTURER=%s\n", make);
				fprintf(output, "MODELNAME=%s\n", model);
				fprintf(output, "MODEL=%s\n", model);
				fprintf(output, "DESCRIPTION=%s %s (Foomatic + %s)\n", make, model, name);
				if (recomm[0] && strcmp(name, recomm) == 0)
					fprintf(output, "RECOMMANDED=yes\n");
				if (comment[0] || comment2[0])
				{
					fprintf(output, "DRIVERCOMMENT=");
					if (comment2[0])
					{
						fprintf(output, "&lt;h3&gt;Printer note&lt;/h3&gt;%s", comment2);
					}
					if (comment[0])
						fprintf(output, "&lt;h3&gt;General driver note&lt;/h3&gt;%s", comment);
					fprintf(output, "\n");
				}
				if ( pnpmake[0] )
					fprintf( output, "PNPMANUFACTURER=%s\n", pnpmake );
				if ( pnpmodel[0] )
					fprintf( output, "PNPMODEL=%s\n", pnpmodel );
				fprintf(output, "\n");
			}
			else if (strcmp(tag, "/printers") == 0)
				break;
		}
	}
	fclose(drFile);
	return 1;
}

void initMatic(const char *base)
{
	char	drPath[256];
	char	drFile[256];
	DIR	*foodir;
	struct dirent	*d;
	struct stat	st;

	if (strstr(base, "foomatic") == NULL)
		return;

	snprintf(drPath, 256, "%s/driver", base);
	foodir = opendir(drPath);
	if (foodir == NULL)
		return;
	while ((d = readdir(foodir)) != NULL)
	{
		snprintf(drFile, 256, "foomatic:%s/%s", drPath, d->d_name);
		if (stat(drFile+9, &st) != 0)
			continue;
		else if (!S_ISREG(st.st_mode))
			continue;
		addFile(drFile);
	}
	closedir(foodir);
}

void initFoomatic(void)
{
	registerHandler("foomatic:", initMatic, parseMaticFile);
}

int execute(int argc, char *argv[])
{
	FILE	*dbFile;
	int	i;
	char	*c, *d;

	/* open output driver DB file */
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

	/* init parsing */
	c = argv[1];
	do
	{
		d = strchr(c, ':');
		if (d != NULL)
			*d = 0;
		if (strncmp(c, "module:", 7) == 0)
		{
			addLib(c+7);
		}
		else
		{
			for (i=0; i<nhandlers; i++)
			{
				(*(handlers[i]->init))(c);
			}
		}
		if (d != NULL)
			c = d+1;
	} while (d && *c);


	/* do actual parsing */
	fprintf(stdout, "%d\n", nfiles);
	fflush(stdout);
	for (i=0; i<nfiles; i++)
	{
		int	hi;
		for (hi=0; hi<nhandlers; hi++)
			if (strncmp(files[i], handlers[hi]->name, handlers[hi]->namelen) == 0)
			{
				handlers[hi]->parse(files[i]+handlers[hi]->namelen, dbFile);
				break;
			}
		fprintf(stdout, "%d\n", i);
		fflush(stdout);
	}

	/* free everything */
	freeFiles();
	freeHandlers();
	freeLibs();
	if (dbFile != stdout)
		fclose(dbFile);

	return 0;
}

/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <goffioul@imec.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define BUFFER_SIZE	1024
#define WORD_SIZE	256

int parseOverview(const char *dbdir, FILE *out);

int main(int argc, char *argv[])
{
	FILE	*out;

	if (argc != 3)
	{
		fprintf(stderr,"usage: make_driver_db_foomatic <dbdirectory> <dbfilename>\n");
		exit(-1);
	}
	out = fopen(argv[2],"w");
	if (out == NULL)
	{
		fprintf(stderr,"Unable to open DB file: %s\n",argv[2]);
		exit(-1);
	}
	/* first parse RHS driver DB */
	if (!parseOverview(argv[1], out))
		fprintf(stderr,"Unable to parse printer DB\n");
	return 0;
}

char* skipSpaces(char *c)
{
	char 	*cc = c;
	while (cc && *cc && isspace(*cc)) cc++;
	return cc;
}

int nextEntity(char *s, FILE *in)
{
	char	c, p[WORD_SIZE] = {0};
	int	index, istag;

	index = 0;
	do
	{
		c = fgetc(in);
	} while (isspace(c) && c != EOF);
	if (c == EOF)
		return -1;
	if (c == '<')
	{
		istag = 1;
		c = fgetc(in);
		if (c != '/')
			ungetc(c, in);
	}
	else
	{
		istag = 0;
		p[index++] = c;
	}
	while (index < WORD_SIZE)
	{
		c = fgetc(in);
		if (c != '>' && c != '<')
			p[index++] = c;
		else
		{
			if (c == '<')
				ungetc(c, in);
			break;
		}
	}
    
    p[WORD_SIZE - 1] = 0;
	strcpy(s, p);
	return istag;
}

void discardTag(const char *tagname, FILE *in)
{
	char	entity[WORD_SIZE];
	int		istag;

	while ((istag=nextEntity(entity, in)) != -1)
	{
		if (istag == 1 && strcmp(tagname, entity) == 0)
			break;
	}
}

void clearDrivers(char **d)
{
	int	index = 0;

	while (d[index])
	{
		free(d[index]);
		d[index++] = 0;
	}
}

int parseOverview(const char *dbdir, FILE *out)
{
	FILE	*in;
	char	printerid[WORD_SIZE], manu[WORD_SIZE], model[WORD_SIZE], tag[WORD_SIZE], driver[WORD_SIZE];
	char	*drivers[20] = {0};
	int		index = 0;
	char	cmd[BUFFER_SIZE] = {0};

	snprintf(cmd, BUFFER_SIZE, "foomatic-combo-xml -O -l %s", dbdir);
	in = popen(cmd, "r");
	if (in == NULL)
		return 0;
	if (nextEntity(tag, in) == 1 && strcmp(tag, "overview") == 0)
	{
		printf("-1\n");
		while (nextEntity(tag, in) == 1)
		{
			if (strcmp(tag, "overview") == 0 || strcmp(tag, "printer") != 0)
				break;
			strcpy(printerid, "");
			strcpy(manu, "");
			strcpy(model, "");
			clearDrivers(drivers);
			while (nextEntity(tag, in) == 1)
			{
				if (strcmp(tag, "id") == 0) nextEntity(printerid, in);
				else if (strcmp(tag, "make") == 0) nextEntity(manu, in);
				else if (strcmp(tag, "model") == 0) nextEntity(model, in);
				else if (strcmp(tag, "drivers") == 0)
				{
					index = 0;
					while (nextEntity(tag, in) == 1 && strcmp(tag, "driver") == 0 && index < 20)
					{
						nextEntity(driver, in);
						drivers[index++] = strdup(driver);
						nextEntity(driver, in);
					}
					continue;
				}
				else if (strcmp(tag, "printer") == 0)
				{
					printf("%s %s\n", manu, model);
					index = 0;
					while (drivers[index])
					{
						fprintf(out, "FILE=%s|%s\n", printerid, drivers[index]);
						fprintf(out, "MANUFACTURER=%s\n", manu);
						fprintf(out, "MODELNAME=%s\n", model);
						fprintf(out, "MODEL=%s\n", model);
						fprintf(out, "DESCRIPTION=%s %s (%s)\n", manu, model, drivers[index]);
						fprintf(out, "\n");
						index++;
					}
					break;
				}
				else
				{
					discardTag(tag, in);
					continue;
				}
				nextEntity(tag, in);
			}
		}
	}
	if (pclose(in) == 0)
		index = 1;
	else
		index = 0;
	return index;
}

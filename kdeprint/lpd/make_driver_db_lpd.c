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

#include <config.h>

#define BUFFER_SIZE	1024

int parseRhsPrinterDb(const char *filename, FILE *out);

int main(int argc, char *argv[])
{
	FILE	*out;

	if (argc != 3)
	{
		fprintf(stderr,"usage: make_driver_db_lpd <dbdirectory> <dbfilename>\n");
		exit(-1);
	}
	out = fopen(argv[2],"w");
	if (out == NULL)
	{
		fprintf(stderr,"Unable to open DB file: %s\n",argv[2]);
		exit(-1);
	}
	/* first parse RHS driver DB */
	if (!parseRhsPrinterDb("/usr/lib/rhs/rhs-printfilters/printerdb",out))
		fprintf(stderr,"Unable to parse RHS DB file\n");
	return 0;
}

char* skipSpaces(char *c)
{
	char 	*cc = c;
	while (cc && *cc && isspace(*cc)) cc++;
	return cc;
}

int parseRhsPrinterDb(const char *filename, FILE *out)
{
	FILE	*in;
	char 	buffer[BUFFER_SIZE], *c;

	in = fopen(filename,"r");
	if (in == NULL)
		return 0;
	while (fgets(buffer,BUFFER_SIZE,in))
	{
		c = skipSpaces(buffer);			/* skip leading white spaces */
		if (c == NULL || *c == '#')		/* empty line or comment line */
			continue;
		if (strncmp(c,"StartEntry:",11) == 0)	/* start a new entry */
		{
			fprintf(out,"\n");
			fprintf(out,"FILE=%s\n",filename);
			c = skipSpaces(c+11);
			if (c)
				fprintf(out,"MODELNAME=%s",c);
		}
		else if (strncmp(c,"Description:",12) == 0)
		{
			char 	*c1, *c2;
			c1 = strchr(c+12,'{');
			c2 = strchr(c+12,'}');
			if (c1 && c2)
			{
				char 	model[BUFFER_SIZE], manuf[BUFFER_SIZE];
				char 	*c3;

				*c2 = 0;
				c1++;
				c3 = strchr(c1,' ');
				if (c3)
				{
					*c3 = 0;
					c3++;
					strlcpy(manuf,c1, sizeof(manuf));
					strlcpy(model,c3, sizeof(model));
				}
				else
				{
					strlcpy(model,c1, sizeof(model));
					strlcpy(manuf,"PrintTool (RH)", sizeof(manuf));
				}
				fprintf(out,"MANUFACTURER=%s\n",manuf);
				fprintf(out,"MODEL=%s\n",model);
				fprintf(out,"DESCRIPTION=%s %s\n",manuf,model);
			}
		}
	}
	return 1;
}

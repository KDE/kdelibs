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

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <cups/cups.h>
#include <cups/ipp.h>

#define BUFSIZE		1024
#define BUFSIZE2	32

/* global variables */
char	passwd[BUFSIZE2] = {0};
int	pwd_asked = 0;

/* utility functions */
void error(const char* msg)
{
	fprintf(stderr, "%s\n", msg);
	exit(-1);
}

void usage()
{
	fprintf(stderr, "usage: cupsdoprint [-H host[:port]][-P dest][-J name][-o opt=value[,...]][-U login[:password]] files...\n");
	exit(-1);
}

const char* getPasswordCB(const char* prompt)
{
	if (pwd_asked != 0 || passwd[0] == 0)
	{
		int	len = 0;
		char	buf[256], *c;

		fprintf(stderr, "?password:%s\n", cupsUser());
		fgets(buf, 256, stdin);
		len = strlen(buf);
		while (len > 0 && isspace(buf[len-1]))
			buf[--len] = 0;
		pwd_asked = 1;
		if (len == 0)
			return NULL;
		if ((c=strchr(buf, ':')) != NULL)
		{
			*c = 0;
			cupsSetUser(buf);
			strncpy(passwd, ++c, BUFSIZE2);
		}
		else
			strncpy(passwd, buf, BUFSIZE2);
	}
	return passwd;
}

/* main function */
int main(int argc, char* argv[])
{
	int	c, port = -1;
	char	printer[BUFSIZE] = {0}, jobname[BUFSIZE] = {0}, host[BUFSIZE] = {0};
	char	login[BUFSIZE2] = {0};
	char	*a;
	cups_option_t	*options = NULL;
	int		num_options = 0;
	char*	files[100] = {0};
	int	num_files = 0;
	int	jobID = 0;

	while ((c=getopt(argc, argv, "P:J:H:o:U:?")) != -1)
	{
		switch (c)
		{
			case 'P':
				strncpy(printer, optarg, BUFSIZE);
				if ((a=strchr(printer, '/')) != NULL)
					error("This utility doesn't support printer instances");
				break;
			case 'J':
				strncpy(jobname, optarg, BUFSIZE);
				break;
			case 'H':
				strncpy(host, optarg, BUFSIZE);
				if ((a=strchr(host, ':')) != NULL)
				{
					*a = 0;
					port = atoi(++a);
					if (port == 0)
						error("Wrong port value");
				}
				break;
			case 'U':
				strncpy(login, optarg, BUFSIZE2);
				if ((a=strchr(login, ':')) != NULL)
				{
					*a = 0;
					strncpy(passwd, ++a, BUFSIZE2);
				}
			case 'o':
				num_options = cupsParseOptions(optarg, num_options, &options);
				break;
			case '?':
			default:
				usage();
				break;
		}
	}

	/* check the printer */
	if (printer[0] == 0)
	{
		if (getenv("PRINTER") != NULL)
			strncpy(printer, getenv("PRINTER"), BUFSIZE);
		else
			error("No printer specified (and PRINTER variable is empty)");
	}

	/* CUPS settings */
	if (host[0] != 0) cupsSetServer(host);
	if (port > 0) ippSetPort(port);
	if (login[0] != 0) cupsSetUser(login);
	cupsSetPasswordCB(getPasswordCB);

	/* check for files */
	if (optind < 1 || optind >= argc)
		error("This utility doesn't support printing from STDIN");
	else
		for (c=optind; c<argc; c++)
		{
			if (access(argv[c], R_OK) != 0)
			{
				fprintf(stderr, "%s: ", argv[c]);
				error("file not found or not readable");
			}
			else
				files[num_files++] = strdup(argv[c]);
		}

	/* print files */
	jobID = cupsPrintFiles(printer, num_files, files, jobname, num_options, options);
	if (jobID <= 0)
		error(ippErrorString(cupsLastError()));

	return 0;
}

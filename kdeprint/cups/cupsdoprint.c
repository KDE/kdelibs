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

#include <config.h>

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <cups/cups.h>
#include <cups/ipp.h>

#define BUFSIZE		1024
#define BUFSIZE2	32

#define	USE_LOG		0

/* global variables */
char	passwd[BUFSIZE2] = {0};
int	pwd_asked = 0;
#if USE_LOG
FILE	*debugF = NULL;
#endif

/* utility functions */
void error(const char* msg)
{
	fprintf(stderr, "%s\n", msg);
#if USE_LOG
	if (debugF != NULL) fclose(debugF);
#endif
	exit(-1);
}

void usage()
{
	error("usage: cupsdoprint [-H host[:port]][-P dest][-J name][-o opt=value[,...]][-U login[:password]] files...");
}

static char * shell_quote(const char *s)
{
   char *result;
   char *p;
   p = result = malloc(strlen(s)*4+3);
   *p++ = '\'';
   while(*s)
   {
     if (*s == '\'')
     {
        *p++ = '\'';
        *p++ = '\\';
        *p++ = '\'';
        *p++ = '\'';
	s++;
     }
     else
     {
        *p++ = *s++;
     }
   }
   *p++ = '\'';
   *p = '\0';
   return result;
}

const char* getPasswordCB(const char* prompt)
{
	char buf[ 256 ] = {0}, *c;
	char *_user = shell_quote( cupsUser() ), *_passwd = NULL;
	FILE *output;

	snprintf( buf, sizeof( buf )-1, "dcop kded kdeprintd requestPassword %s %s %d %d",
			_user,
			cupsServer(),
			ippPort(),
			pwd_asked );
	free( _user );
	_user = NULL;
	output = popen( buf, "r" );
	if ( output != NULL )
	{
		while ( fgets( buf, sizeof( buf )-1, output ) )
		{
			_user = _passwd = NULL;
			if ( strcmp( buf, "::" ) != 0 )
			{
				c = strchr( buf, ':' );
				if ( c != NULL )
				{
					*c = '\0';
					_user = buf;
					_passwd = ++c;
					c = strchr( c, ':' );
					if ( c != NULL )
					{
						*c = '\0';
						/* retrieve password sequence number */
						pwd_asked = atoi( ++c );
						/* update CUPS with current username */
						cupsSetUser( _user );
						/* copy password to a non temporary location */
						strlcpy( passwd, _passwd, BUFSIZE2 );
						_passwd = passwd;
					}
					else
						_passwd = NULL;
				}
			}
		}
		pclose( output );
	}
	else
		return NULL;

	/* erase buffer containing unencrypted password, for security */
	memset( buf, 0, 256 );

	/* if OK, _passwd should point to global passwd variable, otherwise it should be NULL */
	return _passwd;
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

#if USE_LOG
	debugF = fopen("/tmp/cupsdoprint.debug","w");
	if (debugF == NULL)
		error("unable to open log file");
#endif

	while ((c=getopt(argc, argv, "P:J:H:o:U:?")) != -1)
	{
#if USE_LOG
		fprintf(debugF,"%c: %s\n",c,optarg);
#endif
		switch (c)
		{
			case 'P':
				strlcpy(printer, optarg, BUFSIZE);
				if ((a=strchr(printer, '/')) != NULL)
					error("This utility doesn't support printer instances");
				break;
			case 'J':
				strlcpy(jobname, optarg, BUFSIZE);
				break;
			case 'H':
				strlcpy(host, optarg, BUFSIZE);
				if ((a=strchr(host, ':')) != NULL)
				{
					*a = 0;
					port = atoi(++a);
					if (port == 0)
						error("Wrong port value");
				}
				break;
			case 'U':
				strlcpy(login, optarg, BUFSIZE2);
				if ((a=strchr(login, ':')) != NULL)
				{
					*a = 0;
					strlcpy(passwd, ++a, BUFSIZE2);
				}
				break;
			case 'o':
#if USE_LOG
				fprintf(debugF,"Parsing options (n=%d)\n",num_options);
#endif
				num_options = cupsParseOptions(optarg, num_options, &options);
#if USE_LOG
				fprintf(debugF,"Options parsed (n=%d)\n",num_options);
#endif
				break;
			case '?':
			default:
				usage();
				break;
		}
	}

	/* check the printer */
	if (!*printer)
	{
		printer[BUFSIZE-1] = '\0';
		if (getenv("PRINTER") != NULL)
			strlcpy(printer, getenv("PRINTER"), BUFSIZE-1);
		else
			error("No printer specified (and PRINTER variable is empty)");
	}

	/* CUPS settings */
	if (host[0] != 0) cupsSetServer(host);
	if (port > 0) ippSetPort(port);
	if (login[0] != 0) cupsSetUser(login);
	if (jobname[0] == 0) strcpy(jobname,"KDE Print System");
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

#if USE_LOG
	fprintf(debugF,"Processed options:\n");
	for (c=0; c<num_options; c++)
		fprintf(debugF,"%s = %s\n",options[c].name,options[c].value);
#endif
	/* print files */
	jobID = cupsPrintFiles(printer, num_files, files, jobname, num_options, options);
	/* erase unemcrypted password for security */
	memset( passwd, 0, BUFSIZE2 );
	/* check job creation status */
	if (jobID <= 0)
		error(ippErrorString(cupsLastError()));

#if USE_LOG
	fclose(debugF);
#endif
	return 0;
}

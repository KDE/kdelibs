/*
  This file is part of the KDE libraries
  Copyright (c) 1999 Waldo Bastian <bastian@kde.org>
            (c) 1999 Mario Weilguni <mweilguni@sime.com>
            (c) 2001 Lubos Lunak <l.lunak@kde.org>
	    (c) 2003 Stephan Kulow <coolo@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License version 2 as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
  Boston, MA 02111-1307, USA.
*/

#include <config.h>
#include <limits.h>
#include "ltdl.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    char path[PATH_MAX + 1];
    char target[PATH_MAX + 1];
    char *p;

    if (argv[0][0] != '/')
    {
	if (getcwd(path, PATH_MAX)) 
	{
	    strncat(path, "/", PATH_MAX);
	    strncat(path, argv[0], PATH_MAX);
	} else {
	    perror("getpwd");
	    return 1;
	}
    } 
    else 
    {
	strncpy(path, argv[0], PATH_MAX);
    }

    p = path + strlen(path);
    while (p > path)
    {
	if (*p == '/') break;
	p--;
    }

    if ( *p == '/')
	*p = 0;

    strncpy(target, p + 1, PATH_MAX);

    strncat(path, "/../lib/kde3/", PATH_MAX);
    strncat(path, target, PATH_MAX);
    strncat(path, ".la", PATH_MAX);
    
    if (!access(path, R_OK)) {
	
	lt_dlhandle handle;
	lt_ptr sym;
	int (*func)(int, char *[]);
	int ret;

	if (lt_dlinit())
	{
	    const char * errMsg = lt_dlerror();
	    fprintf(stderr, "can't initialize dynamic loading: %s\n", 
		    errMsg ? errMsg : "(null)" );
	}

	handle = lt_dlopen(path);

	if ( !handle )
	{
	    const char* errMsg = lt_dlerror();
	    fprintf(stderr, "Can't open %s - error: %s\n",
		    path, errMsg ? errMsg : "(null)");
	    return 1;
	} 	
	sym = lt_dlsym( handle, "kdemain");
	if (!sym )
        {
	    const char * errMsg = lt_dlerror();
	    fprintf(stderr, "Can't find 'kdemain' in '%s'\n%s",
		    path, errMsg ? errMsg : "(null)");
	    return 1;
	}
	func = (int (*)(int, char *[])) sym;
	ret = func(argc, argv);
	lt_dlexit();
	return ret;

    } else {
	fprintf(stderr, "Can't access %s\n", path);
	return 1;
    }

    return 0;
}
   

/*
  This file is part of the KDE libraries
  Copyright (c) 2003 Stephan Kulow <coolo@kde.org>
		2003 Michael Matz <matz@kde.org>

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

    if (!strchr (argv[0], '/'))
      {
        /* Name without any dirs.  Search $PATH.  */
	char *dirs = getenv ("PATH");
	char *d = dirs;
	path[0] = 0;
	/* So, it's slow with all the strcpy and strcats.  Who cares?  */
	while (d)
	  {
	    dirs = d;
	    d = strchr (dirs, ':');
	    if (d == dirs  /* "::" or ':' at begin --> search pwd */
	        || !*dirs) /* ':' at end --> ditto */
	      strncpy (path, argv[0], PATH_MAX); 
	    else
	      {
		if (d)
		  {
		    strncpy (path, dirs, d - dirs);
		    path[d - dirs] = '/';
		    path[d - dirs + 1] = 0;
		  }
		else
		  {
		    strncpy (path, dirs, PATH_MAX);
		    strncat (path, "/", PATH_MAX);
		  }
		strncat (path, argv[0], PATH_MAX);
	      }
	    path[PATH_MAX] = 0;
	    if (!access (path, X_OK))
	      break;
	    if (d)
	      d++;
	  }
      }
    else
      strncpy (path, argv[0], PATH_MAX);

    if (path[0] != '/')
      {
        /* Relative path, prepend pwd.  */
	if (getcwd(path, PATH_MAX)) 
	  {
	    strncat(path, "/", PATH_MAX);
	    strncat(path, argv[0], PATH_MAX);
	  } else {
	    perror("getpwd");
	    return 1;
	  }
      }

    if (!*path || access (path, X_OK))
      {
	fprintf(stderr, "Can't find myself (I'm %s).\n", argv[0]);
	return 1;
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

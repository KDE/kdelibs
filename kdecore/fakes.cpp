/* This file is part of the KDE libraries
   Copyright (c) 2000 The KDE Project

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

#ifndef HAVE_SETENV

#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int setenv(const char *name, const char *value, int overwrite) {
    int i;
    char * a;

    if (!overwrite && getenv(name)) return 0;

    i = strlen(name) + strlen(value) + 2;
    a = (char*)malloc(i);
    if (!a) return 1;
    
    strcpy(a, name);
    strcat(a, "=");
    strcat(a, value);
    
    return putenv(a);
}
#endif

#ifndef HAVE_UNSETENV

#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif

#include <string.h>
#include <stdlib.h>

int unsetenv(const char * name) {
    int i;
    char * a;

    i = strlen(name) + 2;
    a = (char*)malloc(i);
    if (!a) return 1;
    
    strcpy(a, name);
    strcat(a, "=");
    
    return putenv(a);
}

#endif

#ifndef HAVE_USLEEP

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

extern "C" {

void usleep(unsigned int usec) {
        struct timeval _usleep_tv;
        _usleep_tv.tv_sec = usec/1000000;
        _usleep_tv.tv_usec = usec%1000000;
        select(0,0,0,0,&_usleep_tv);
}

}

#endif

#if !defined(HAVE_GETDOMAINNAME)

#include <sys/utsname.h> 
#include <netdb.h>
#include <strings.h>
#include <errno.h>
#include <stdio.h>

int getdomainname(char *name, size_t len)
{
        struct utsname uts;
        struct hostent *hent;
        int rv = -1;

        if (name == 0L)
          errno = EINVAL;
        else
        {               
                name[0] = '\0';
                if (uname(&uts) >= 0)
                {
                        if ((hent = gethostbyname(uts.nodename)) != 0L)
                        {
                                char *p = strchr(hent->h_name, '.');
                                if (p != 0L)
                                {
                                        ++p;
                                        if (strlen(p) > len-1)
                                          errno = EINVAL;
                                        else
                                        {
                                                strcpy(name, p);
                                                rv = 0;
                                        }
                                }
                        }
                }
        }
        return rv;
}


#endif

#ifndef HAVE_RANDOM
long int random()
{
    return lrand48();
}

void srandom(unsigned int seed)
{
    srand48(seed);
}
#endif



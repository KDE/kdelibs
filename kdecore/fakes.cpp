
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef HAVE_FUNC_SETENV

#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif

#include <string.h>
#include <stdlib.h>

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

void usleep(unsigned int usec) {
        struct timeval _usleep_tv;
        _usleep_tv.tv_sec = usec/1000000;
        _usleep_tv.tv_usec = usec%1000000;
        select(0,0,0,0,&_usleep_tv);
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

        if (name == NULL)
          errno = EINVAL;
        else
        {               
                name[0] = '\0';
                if (uname(&uts) >= 0)
                {
                        if ((hent = gethostbyname(uts.nodename)) != NULL)
                        {
                                char *p = strchr(hent->h_name, '.');
                                if (p != NULL)
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

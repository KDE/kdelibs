#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef HAVE_FUNC_SETENV
int setenv(const char *name, const char *value, int overwrite) ;
#endif      

#ifndef HAVE_FUNC_USLEEP

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

void usleep(int usec) {
        struct timeval _usleep_tv;
        _usleep_tv.tv_sec = usec/1000000;
        _usleep_tv.tv_usec = usec%1000000;
        select(0,0,0,0,&_usleep_tv);
}
#endif


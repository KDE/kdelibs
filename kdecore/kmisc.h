#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef HAVE_FUNC_SETENV
int setenv(const char *name, const char *value, int overwrite) ;
#endif      

#ifndef HAVE_FUNC_USLEEP
void usleep(int usec);
#endif


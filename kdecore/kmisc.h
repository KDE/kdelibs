#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef HAVE_FUNC_SETENV
int setenv(const char *name, const char *value, int overwrite) ;
#endif      

#ifdef HAVE_GETDOMAINNAME
extern "C" {
int getdomainname (char *Name, int Namelen);
}
#endif


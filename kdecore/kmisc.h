#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef HAVE_FUNC_SETENV
extern "C" {
 int setenv(const char *name, const char *value, int overwrite) ;
}
#endif      

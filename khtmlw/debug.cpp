#include <stdio.h>  
#include <stdarg.h>      
#include "html.h"

#ifdef MARTINSDEBUG
void debugM( const char *msg, ...)
{
    va_list ap;
    va_start( ap, msg );                // use variable arg list
    vfprintf( stdout, msg, ap );
    va_end( ap );
#else
void debugM(const char *, ... )
{
#endif
}       


#ifndef _KDEBUG_H
#define _KDEBUG_H

void kdebug( unsigned short, unsigned int, const char* );

#ifndef NDEBUG
#define KDEBUG( Level, Area, String ) kdebug( (Level), (Area), (String) )
#else
#define KDEBUG( Level, Area, String )
#endif

#define KDEBUG_INFO 0
#define KDEBUG_WARN 1
#define KDEBUG_ERROR 2
#define KDEBUG_FATAL 3

#endif


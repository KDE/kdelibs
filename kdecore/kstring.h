#ifndef __kstring_h__
#define __kstring_h__

#include <qstring.h>

QString& operator<<( QString&, short );
QString& operator<<( QString&, ushort );
QString& operator<<( QString&, int );
QString& operator<<( QString&, uint );
QString& operator<<( QString&, long );
QString& operator<<( QString&, ulong );
QString& operator<<( QString&, float );
QString& operator<<( QString&, double );
QString& operator<<( QString&, const char* );

/* replaces QString::sprintf, where it's not sure, that 256
   bytes are enough for the resulting string.
   
   ksprintf adds to the 256 default bytes the len of every
   used string. 
   */
void ksprintf(QString *str, const char *fmt, ...);

#endif

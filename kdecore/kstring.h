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

#endif

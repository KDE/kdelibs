#include "kstring.h"
#include <stdio.h>
#include <stdarg.h>

QString& operator<<( QString& _str, short _v )
{
    QString tmp;
    tmp.setNum( _v );
    _str += tmp.data();
    return _str;
}

QString& operator<<( QString& _str, ushort _v )
{
    QString tmp;
    tmp.setNum( _v );
    _str += tmp.data();
    return _str;
}

QString& operator<<( QString& _str, int _v )
{
    QString tmp;
    tmp.setNum( _v );
    _str += tmp.data();
    return _str;
}

QString& operator<<( QString& _str, uint _v )
{
    QString tmp;
    tmp.setNum( _v );
    _str += tmp.data();
    return _str;
}

QString& operator<<( QString& _str, long _v )
{
    QString tmp;
    tmp.setNum( _v );
    _str += tmp.data();
    return _str;
}

QString& operator<<( QString& _str, ulong _v )
{
    QString tmp;
    tmp.setNum( _v );
    _str += tmp.data();
    return _str;
}

QString& operator<<( QString& _str, float _v )
{
    QString tmp;
    tmp.setNum( _v );
    _str += tmp.data();
    return _str;
}

QString& operator<<( QString& _str, double _v )
{
    QString tmp;
    tmp.setNum( _v );
    _str += tmp.data();
    return _str;
}

QString& operator<<( QString& _str, const QString &_v )
{
    _str += _v;
    return _str;
}

void ksprintf(QString *str, const char *fmt, ...)
{
    va_list ap;
    va_start( ap, fmt );

    int i = 0;
    int len = 255; // default is the same as in QString::sprintf
    int dummy; // this ugly hack is just to avoid warnings

    while (fmt[i+1]) {

	if (fmt[i] == '%' && fmt[i+1] != '%' ) { // argument
	    if (fmt[i+1] == 's') {
		const char *str = va_arg( ap, const char*);
		len += strlen(str);
	    } else
		dummy = va_arg (ap, int); // take it out of the arg list
	} else // jump over "%%"
	    if (fmt[i] == '%' && fmt[i+1] == '%') i++;

	i++;
    }

    va_end( ap); // oh god, what I hope, that restarting is portable

    char *tmp_data = new char[len]; // make enough space

    va_start( ap, fmt ); // the rest does the libc for us
    vsprintf( tmp_data, fmt, ap );
    va_end( ap );

    *str = tmp_data; // makes deep copy, that is also resized

    delete [] tmp_data; // clear the heap
}

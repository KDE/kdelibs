#include "kstring.h"

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

QString& operator<<( QString& _str, const char* _v )
{
    _str += _v;
    return _str;
}

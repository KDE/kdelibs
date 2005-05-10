#ifndef __kdatastream__h
#define __kdatastream__h

#include <qdatastream.h>

//Compatibility classes for QCString, QCStringArray, that provide marshalling,
//and type ID compatibility
class DCOPCString: public QByteArray
{
public:
    DCOPCString()
    {}

    DCOPCString(int size): QByteArray(size)
    {}


    DCOPCString(const char* str): QByteArray(str)
    {}

    DCOPCString(const QByteArray& array): QByteArray(array)
    {}

    DCOPCString& operator=(const QByteArray& ba)
    {
        QByteArray::operator=(ba); return *this;
    }

    DCOPCString& operator=(const char* data)
    {
        QByteArray::operator=(data); return *this;
    }

    DCOPCString& operator=(const DCOPCString& ds)
    {
        QByteArray::operator=(ds); return *this;
    }
};

typedef QList<DCOPCString> DCOPCStringList; //Marshalls appropriately..

inline const char* dcopTypeName( const DCOPCStringList& ) { return "QCStringList"; }
inline const char* dcopTypeName( const DCOPCString&     ) { return "QCString";     }


inline QDataStream & operator << (QDataStream & str, const DCOPCString& s)
{
    str << s.length() + 1;
    str.writeRawBytes(s.data(), s.length());
    str.writeRawBytes("\0",     1);

    return str;
}

inline QDataStream & operator >> (QDataStream & str, DCOPCString& s)
{
    quint32 length;
    str >> length;
    s.resize(length);

    if (length)
    {
        str.readRawBytes(s.data(), s.length());
        s.resize(length - 1); //Drop the null
    }
        
    return str;
}



#if QT_VERSION < 0x030200 && !defined(Q_WS_WIN) && !defined(Q_WS_MAC)
inline QDataStream & operator << (QDataStream & str, long long int ll)
{
  Q_UINT32 l1,l2;
  l1 = ll & 0xffffffffLL;
  l2 = ll >> 32;
  str << l1 << l2;
  return str;
}

inline QDataStream & operator >> (QDataStream & str, long long int&ll)
{
  Q_UINT32 l1,l2;
  str >> l1 >> l2;
  ll = ((unsigned long long int)(l2) << 32) + (long long int) l1;
  return str;
}

inline QDataStream & operator << (QDataStream & str, unsigned long long int ll)
{
  Q_UINT32 l1,l2;
  l1 = ll & 0xffffffffLL;
  l2 = ll >> 32;
  str << l1 << l2;
  return str;
}

inline QDataStream & operator >> (QDataStream & str, unsigned long long int &ll)
{
  Q_UINT32 l1,l2;
  str >> l1 >> l2;
  ll = ((unsigned long long int)(l2) << 32) + (unsigned long long int) l1;
  return str;
}
#endif

#endif

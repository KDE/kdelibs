#ifndef kdatastream__h
#define kdatastream__h

#include <QSysInfo>
#include <qdatastream.h>

//Compatibility classes for QCString, QCStringArray, that provide marshalling,
//and type ID compatibility
class DCOPCString: public QByteArray
{
public:
    DCOPCString()
    {}

    DCOPCString(int size): QByteArray(size, '\0')
    {}


    DCOPCString(const char* str): QByteArray(str)
    { squeezeTrailNulls(); }

    DCOPCString(const QByteArray& array): QByteArray(array)
    { squeezeTrailNulls(); }

    DCOPCString& operator=(const QByteArray& ba)
    {
        QByteArray::operator=(ba);
        squeezeTrailNulls();
        return *this;
    }

    DCOPCString& operator=(const char* data)
    {
        QByteArray::operator=(data); return *this;
    }

    DCOPCString& operator=(const DCOPCString& ds)
    {
        QByteArray::operator=(ds); return *this;
    }
private:
    void squeezeTrailNulls()
    {
        while (length() && data()[length() - 1] == '\0')
            truncate(length() - 1);
    }
};

typedef QList<DCOPCString> DCOPCStringList; //Marshalls appropriately..

//inline const char* dcopTypeName( const DCOPCStringList& ) { return "QCStringList"; }

/**
 Compatibility with Qt/KDE3: unfortunately, the size varies between platforms.
 To make things even more fun, the encoding on 64-bit platforms is -different-
 from the long encoding. Whee!
*/
inline QDataStream & operator << (QDataStream & str, unsigned long val)
{
    QByteArray buf(sizeof(unsigned long), '\0');
    qMemCopy(buf.data(), (unsigned char*)&val, sizeof(unsigned long));

    //Swap if need be
    if ((int)str.byteOrder() != QSysInfo::ByteOrder)
    {
        for (int low = 0; low <= (buf.size()/2 - 1); ++low)
            qSwap(buf.data()[low], buf.data()[buf.size() - 1 - low]);
    }

    //Write out.
    str.writeRawData(buf.data(), buf.size());
    return str;
}

inline QDataStream & operator >> (QDataStream & str, unsigned long& val)
{
    QByteArray buf(sizeof(unsigned long), '\0');
    str.readRawData(buf.data(), buf.size());

    //Swap if need be
    if ((int)str.byteOrder() != QSysInfo::ByteOrder)
    {
        for (int low = 0; low <= (buf.size()/2 - 1); ++low)
            qSwap(buf.data()[low], buf.data()[buf.size() - 1 - low]);
    }

    //Store in the val
    qMemCopy((unsigned char*)&val, buf.data(), sizeof(unsigned long));

    return str;
}

inline QDataStream & operator << (QDataStream & str, long val)
{
    return str << (unsigned long)val;
}

inline QDataStream & operator >> (QDataStream & str, long& val)
{
    unsigned long tmp;
    (str >> tmp);
    val = (long)tmp;
    return str;
}


inline QDataStream & operator << (QDataStream & str, const DCOPCString& s)
{
    int len = strlen(s.data());
    str << len + 1;
    str.writeRawData(s.data(), len + 1);
    return str;
}

inline QDataStream & operator >> (QDataStream & str, DCOPCString& s)
{
    quint32 length;
    str >> length;
    s.resize(length);

    if (length)
    {
        str.readRawData(s.data(), s.length());
        s.resize(length - 1); //Drop the null
    }
        
    return str;
}

#endif


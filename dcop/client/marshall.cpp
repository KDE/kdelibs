/*****************************************************************
Copyright (c) 2000 Matthias Ettrich <ettrich@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#define KDE_QT_ONLY
#include "../../kdecore/kurl.cpp"

bool mkBool( const QString& s )
{
    if ( s.lower()  == "true" )
	return TRUE;
    if ( s.lower()  == "yes" )
	return TRUE;
    if ( s.lower()  == "on" )
	return TRUE;
    if ( s.toInt() != 0 )
	return TRUE;

    return FALSE;
}

QPoint mkPoint( const QString &str )
{
    const char *s = str.latin1();
    char *end;
    while(*s && !isdigit(*s)) s++;
    int x = strtol(s, &end, 10);
    s = (const char *)end;
    while(*s && !isdigit(*s)) s++;
    int y = strtol(s, &end, 10);
    return QPoint( x, y );
}

QSize mkSize( const QString &str )
{
    const char *s = str.latin1();
    char *end;
    while(*s && !isdigit(*s)) s++;
    int w = strtol(s, &end, 10);
    s = (const char *)end;
    while(*s && !isdigit(*s)) s++;
    int h = strtol(s, &end, 10);
    return QSize( w, h );
}

QRect mkRect( const QString &str )
{
    const char *s = str.latin1();
    char *end;
    while(*s && !isdigit(*s)) s++;
    int p1 = strtol(s, &end, 10);
    s = (const char *)end;
    bool legacy = (*s == 'x');
    while(*s && !isdigit(*s)) s++;
    int p2 = strtol(s, &end, 10);
    s = (const char *)end;
    while(*s && !isdigit(*s)) s++;
    int p3 = strtol(s, &end, 10);
    s = (const char *)end;
    while(*s && !isdigit(*s)) s++;
    int p4 = strtol(s, &end, 10);
    if (legacy)
    {
       return QRect( p3, p4, p1, p2 );
    }
    return QRect( p1, p2, p3, p4 );
}

QColor mkColor( const QString& s )
{
    QColor c;
    c.setNamedColor(s);
    return c;
}

const char *qStringToC(const QCString &s)
{
   if (s.isEmpty())
      return "";
   return s.data();
}

QCString demarshal( QDataStream &stream, const QString &type )
{
    QCString result;

    if ( type == "int" )
    {
        int i;
        stream >> i;
        result.sprintf( "%d", i );
    } else if ( type == "uint" )
    {
        uint i;
        stream >> i;
        result.sprintf( "%d", i );
    } else if ( type == "long" )
    {
        long l;
        stream >> l;
        result.sprintf( "%ld", l );
    } else if ( type == "float" )
    {
        float f;
        stream >> f;
        result.sprintf( "%f", (double)f );
    } else if ( type == "double" )
    {
        double d;
        stream >> d;
        result.sprintf( "%f", d );
    } else if ( type == "bool" )
    {
        bool b;
        stream >> b;
        result = b ? "true" : "false";
    } else if ( type == "QString" )
    {
        QString s;
        stream >> s;
        result = s.local8Bit();
    } else if ( type == "QCString" )
    {
        stream >> result;
    } else if ( type == "QCStringList" )
    {
        return demarshal( stream, "QValueList<QCString>" );
    } else if ( type == "QStringList" )
    {
        return demarshal( stream, "QValueList<QString>" );
    } else if ( type == "QColor" )
    {
        QColor c;
        stream >> c;
        result = c.name().local8Bit();
    } else if ( type == "QSize" )
    {
        QSize s;
        stream >> s;
        result.sprintf( "%dx%d", s.width(), s.height() );
    } else if ( type == "QPoint" )
    {
        QPoint p;
        stream >> p;
        result.sprintf( "+%d+%d", p.x(), p.y() );
    } else if ( type == "QRect" )
    {
        QRect r;
        stream >> r;
        result.sprintf( "%dx%d+%d+%d", r.width(), r.height(), r.x(), r.y() );
    } else if ( type == "QVariant" )
    {
        Q_INT32 type;
        stream >> type;
        return demarshal( stream, QVariant::typeToName( (QVariant::Type)type ) );
    } else if ( type == "DCOPRef" )
    {
        DCOPRef r;
        stream >> r;
        result.sprintf( "DCOPRef(%s,%s)", qStringToC(r.app()), qStringToC(r.object()) );
    } else if ( type == "KURL" )
    {
        KURL r;
        stream >> r;
        result = r.url().local8Bit();
    } else if ( type.left( 11 ) == "QValueList<" )
    {
        if ( (uint)type.find( '>', 11 ) != type.length() - 1 )
            return result;

        QString nestedType = type.mid( 11, type.length() - 12 );

        if ( nestedType.isEmpty() )
            return result;

        Q_UINT32 count;
        stream >> count;

        Q_UINT32 i = 0;
        for (; i < count; ++i )
        {
            QCString arg = demarshal( stream, nestedType );
            if ( arg.isEmpty() )
                continue;

            result += arg;

            if ( i < count - 1 )
                result += '\n';
        }
    } else if ( type.left( 5 ) == "QMap<" )
    {
        int commaPos = type.find( ',', 5 );

        if ( commaPos == -1 )
            return result;

        if ( (uint)type.find( '>', commaPos ) != type.length() - 1 )
            return result;

        QString keyType = type.mid( 5, commaPos - 5 );
        QString valueType = type.mid( commaPos + 1, type.length() - commaPos - 2 );

        Q_UINT32 count;
        stream >> count;

        Q_UINT32 i = 0;
        for (; i < count; ++i )
        {
            QCString key = demarshal( stream, keyType );

            if ( key.isEmpty() )
                continue;

            QCString value = demarshal( stream, valueType );

            if ( value.isEmpty() )
                continue;

            result += key + "->" + value;

            if ( i < count - 1 )
                result += '\n';
        }
    }
    else
    {
       result.sprintf( "<%s>", type.latin1());
    }

    return result;

}

void marshall( QDataStream &arg, QCStringList args, uint &i, QString type )
{
    if (type == "QStringList")
       type = "QValueList<QString>";
    if (type == "QCStringList")
       type = "QValueList<QCString>";
    if( i > args.count() )
    {
	qWarning("Not enough arguments.");
	exit(1);
    }       
    QString s = QString::fromLocal8Bit( args[ i ] );

    if ( type == "int" )
	arg << s.toInt();
    else if ( type == "uint" )
	arg << s.toUInt();
    else if ( type == "unsigned" )
	arg << s.toUInt();
    else if ( type == "unsigned int" )
	arg << s.toUInt();
    else if ( type == "long" )
	arg << s.toLong();
    else if ( type == "long int" )
	arg << s.toLong();
    else if ( type == "unsigned long" )
	arg << s.toULong();
    else if ( type == "unsigned long int" )
	arg << s.toULong();
    else if ( type == "float" )
	arg << s.toFloat();
    else if ( type == "double" )
	arg << s.toDouble();
    else if ( type == "bool" )
	arg << mkBool( s );
    else if ( type == "QString" )
	arg << s;
    else if ( type == "QCString" )
	arg << QCString( args[ i ] );
    else if ( type == "QColor" )
	arg << mkColor( s );
    else if ( type == "QPoint" )
	arg << mkPoint( s );
    else if ( type == "QSize" )
	arg << mkSize( s );
    else if ( type == "QRect" )
	arg << mkRect( s );
    else if ( type == "KURL" )
	arg << KURL( s );
    else if ( type == "QVariant" ) {
	if ( s == "true" || s == "false" )
	    arg << QVariant( mkBool( s ), 42 );
	else if ( s.left( 4 ) == "int(" )
	    arg << QVariant( s.mid(4, s.length()-5).toInt() );
	else if ( s.left( 7 ) == "QPoint(" )
	    arg << QVariant( mkPoint( s.mid(7, s.length()-8) ) );
	else if ( s.left( 6 ) == "QSize(" )
	    arg << QVariant( mkSize( s.mid(6, s.length()-7) ) );
	else if ( s.left( 6 ) == "QRect(" )
	    arg << QVariant( mkRect( s.mid(6, s.length()-7) ) );
	else if ( s.left( 7 ) == "QColor(" )
	    arg << QVariant( mkColor( s.mid(7, s.length()-8) ) );
	else
	    arg << QVariant( s );
    } else if ( type.startsWith("QValueList<")) {
	type = type.mid(11, type.length() - 12);
	QStringList list;
	QString delim = s;
	if (delim == "[")
	   delim = "]";
	if (delim == "(")
	   delim = ")";
	i++;
	QByteArray dummy_data;
	QDataStream dummy_arg(dummy_data, IO_WriteOnly);

	uint j = i;
	uint count = 0;
	// Parse list to get the count
	while (true) {
	    if( j > args.count() )
	    {
		qWarning("List end-delimiter '%s' not found.", delim.latin1());
		exit(1);
	    }
	    if( QString::fromLocal8Bit( args[ j ] ) == delim )
		break;
	    marshall( dummy_arg, args, j, type );
	    count++;
	}
	arg << (Q_UINT32) count;
	// Parse the list for real
	while (true) {
	    if( i > args.count() )
	    {
		qWarning("List end-delimiter '%s' not found.", delim.latin1());
		exit(1);
	    }
	    if( QString::fromLocal8Bit( args[ i ] ) == delim )
		break;
	    marshall( arg, args, i, type );
	}
    } else {
	qWarning( "cannot handle datatype '%s'", type.latin1() );
	exit(1);
    }
    i++;
}

// vim: set noet ts=8 sts=4 sw=4:


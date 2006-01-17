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
#undef KDECORE_EXPORT
#define KDECORE_EXPORT
#include "../../kdecore/kurl.cpp"

bool mkBool( const QString& s )
{
    if ( s.toLower()  == "true" )
	return true;
    if ( s.toLower()  == "yes" )
	return true;
    if ( s.toLower()  == "on" )
	return true;
    if ( s.toInt() != 0 )
	return true;

    return false;
}

QPoint mkPoint( const QString &str )
{
    const char *s = str.toLatin1().constData();
    char *end;
    while(*s && !isdigit(*s) && *s != '-') s++;
    int x = strtol(s, &end, 10);
    s = (const char *)end;
    while(*s && !isdigit(*s) && *s != '-') s++;
    int y = strtol(s, &end, 10);
    return QPoint( x, y );
}

QSize mkSize( const QString &str )
{
    const char *s = str.toLatin1().constData();
    char *end;
    while(*s && !isdigit(*s) && *s != '-') s++;
    int w = strtol(s, &end, 10);
    s = (const char *)end;
    while(*s && !isdigit(*s) && *s != '-') s++;
    int h = strtol(s, &end, 10);
    return QSize( w, h );
}

QRect mkRect( const QString &str )
{
    const char *s = str.toLatin1().constData();
    char *end;
    while(*s && !isdigit(*s) && *s != '-') s++;
    int p1 = strtol(s, &end, 10);
    s = (const char *)end;
    bool legacy = (*s == 'x');
    while(*s && !isdigit(*s) && *s != '-') s++;
    int p2 = strtol(s, &end, 10);
    s = (const char *)end;
    while(*s && !isdigit(*s) && *s != '-') s++;
    int p3 = strtol(s, &end, 10);
    s = (const char *)end;
    while(*s && !isdigit(*s) && *s != '-') s++;
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

const char *qStringToC(const QByteArray &s)
{
   if (s.isEmpty())
      return "";
   return s.data();
}

#ifdef _GNUC
#warning FIX the marshalled types
#endif
DCOPCString demarshal( QDataStream &stream, const QString &type )
{
    DCOPCString result;

    if ( type == "int" || type == "Q_INT32" )
    {
        int i;
        stream >> i;
        result.setNum( i );
    } else if ( type == "uint" || type == "Q_UINT32" || type == "unsigned int" )
    {
        uint i;
        stream >> i;
        result.setNum( i );
    } else if ( type == "long" || type == "long int" )
    {
        qint32 l; // TODO: this is long in kde3!
        stream >> l;
        result.setNum( l );
    } else if ( type == "unsigned long" || type == "unsigned long int" )
    {
        quint32 l; // TODO: this is ulong in kde3!
        stream >> l;
        result.setNum( l );
    } else if ( type == "float" )
    {
        float f;
        stream >> f;
        result.setNum( f, 'f' );
    } else if ( type == "double" )
    {
        double d;
        stream >> d;
        result.setNum( d, 'f' );
    } else if ( type == "Q_INT64" ) {
        qint64 i;
        stream >> i;
        result = QString().sprintf( "%lld", i ).toAscii();
    } else if ( type == "Q_UINT64" ) {
        quint64 i;
        stream >> i;
        result = QString().sprintf( "%llu", i ).toAscii();
    } else if ( type == "bool" )
    {
        bool b;
        stream >> b;
        result = b ? "true" : "false";
    } else if ( type == "QString" )
    {
        QString s;
        stream >> s;
        result = s.toLocal8Bit();
    } else if ( type == "QByteArray") 
    {
        QByteArray ba;
        stream >> ba;
        result  = ba;
    } else if ( type == "QCString" )
    {
        stream >> result;
    } else if ( type == "QCStringList" )
    {
        return demarshal( stream, "QList<QCString>" );
    } else if ( type == "QStringList" )
    {
        return demarshal( stream, "QList<QString>" );
    } else if ( type == "QColor" )
    {
        QColor c;
        stream >> c;
        result = c.name().toLocal8Bit();
    } else if ( type == "QSize" )
    {
        QSize s;
        stream >> s;
        result = QString().sprintf( "%dx%d", s.width(), s.height() ).toAscii();
    } else if ( type == "QPixmap" || type == "QImage" )
    {
        QImage i;
        stream >> i;
        QByteArray ba;
        QBuffer buf( &ba );
        buf.open( QIODevice::WriteOnly );
        i.save( &buf, "XPM" );
        result = ba;
    } else if ( type == "QPoint" )
    {
        QPoint p;
        stream >> p;
        result = QString().sprintf( "+%d+%d", p.x(), p.y() ).toAscii();
    } else if ( type == "QRect" )
    {
        QRect r;
        stream >> r;
        result = QString().sprintf( "%dx%d+%d+%d", r.width(), r.height(), r.x(), r.y() ).toAscii();
    } else if ( type == "QVariant" )
    {
        //OK, this is more than a bit eeky. Since we're in a compat mode,
        //we can't get the name of the variant without unmarshalling it,
        //via QVariant, but we also want to unmarshal it ourselves...
        //So we unmarshal and remarshal via QVariant, and then do it
        //ourselves with type known. Icky, isn't it?
        QVariant var;
        stream >> var;

        QByteArray  buf;
        QDataStream redump( &buf, QIODevice::WriteOnly );
        redump.setVersion(QDataStream::Qt_3_1);
        redump << var;

        QDataStream replayed( buf );
        quint32 id; replayed >> id; //Don't care about this anymore
        

        return demarshal( replayed, var.typeName() );
    } else if ( type == "DCOPRef" )
    {
        DCOPRef r;
        stream >> r;
        result = QString().sprintf( "DCOPRef(%s,%s)", qStringToC(r.app()), qStringToC(r.object()) ).toAscii();
    } else if ( type == "KUrl" )
    {
        KUrl r;
        stream >> r;
        result = r.url().toLocal8Bit();
    } else if ( type.startsWith("QList<") )
    {
        if ( type.indexOf( '>' ) != type.length() - 1 )
            return result;

        QString nestedType = type.mid( strlen("QList<") );
        nestedType.truncate(nestedType.length() - 1);

        if ( nestedType.isEmpty() )
            return result;

        quint32 count;
        stream >> count;

        quint32 i = 0;
        for (; i < count; ++i )
        {
            QByteArray arg = demarshal( stream, nestedType );
            result += arg;

            if ( i < count - 1 )
                result += '\n';
        }
    } else if ( type.startsWith( QLatin1String( "QMap<" ) ) )
    {
        int commaPos = type.indexOf( ',', 5 );

        if ( commaPos == -1 )
            return result;

        if ( type.indexOf( '>', commaPos ) != type.length() - 1 )
            return result;

        QString keyType = type.mid( 5, commaPos - 5 );
        QString valueType = type.mid( commaPos + 1, type.length() - commaPos - 2 );

        quint32 count;
        stream >> count;

        quint32 i = 0;
        for (; i < count; ++i )
        {
            QByteArray key = demarshal( stream, keyType );

            if ( key.isEmpty() )
                continue;

            QByteArray value = demarshal( stream, valueType );

            if ( value.isEmpty() )
                continue;

            result += key + "->" + value;

            if ( i < count - 1 )
                result += '\n';
        }
    }
    else
    {
       result = QString().sprintf( "<%s>", type.toLatin1().data()).toAscii();
    }

    return result;

}

#ifdef _GNUC
#warning FIX the marshalled types
#endif
void marshall( QDataStream &arg, DCOPCStringList args, int &i, QString type )
{
    if (type == "QStringList")
       type = "QList<QString>";
    if (type == "QCStringList")
       type = "QList<QCString>";
    if( i >= args.count() )
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
    else if ( type == "Q_INT32" )
	arg << s.toInt();
    else if ( type == "Q_INT64" ) {
	QVariant qv = QVariant( s );
	arg << qv.toLongLong();
    }
    else if ( type == "Q_UINT32" )
	arg << s.toUInt();
    else if ( type == "Q_UINT64" ) {
	QVariant qv = QVariant( s );
	arg << qv.toULongLong();
    }
    else if ( type == "long" )
	arg << ( qint32 )s.toLong();
    else if ( type == "long int" )
	arg << ( qint32 )s.toLong();
    else if ( type == "unsigned long" )
	arg << ( quint32 )s.toULong();
    else if ( type == "unsigned long int" )
	arg << ( quint32 )s.toULong();
    else if ( type == "float" )
	arg << s.toFloat();
    else if ( type == "double" )
	arg << s.toDouble();
    else if ( type == "bool" )
	arg << mkBool( s );
    else if ( type == "QString" )
	arg << s;
    else if ( type == "QByteArray")
	arg << QByteArray( args[ i ] );
    else if ( type == "QCString" )
        arg << DCOPCString( args[ i ] );
    else if ( type == "QColor" )
	arg << mkColor( s );
    else if ( type == "QPoint" )
	arg << mkPoint( s );
    else if ( type == "QSize" )
	arg << mkSize( s );
    else if ( type == "QRect" )
	arg << mkRect( s );
    else if ( type == "KUrl" )
	arg << KUrl( s );
    else if ( type == "QVariant" ) {
	if ( s == "true" || s == "false" )
	    arg << QVariant( mkBool( s ) );
	else if ( s.startsWith( QLatin1String( "int(" ) ) )
	    arg << QVariant( s.mid(4, s.length()-5).toInt() );
        else if ( s.startsWith( QLatin1String( "QPoint(" ) ) )
	    arg << QVariant( mkPoint( s.mid(7, s.length()-8) ) );
	else if ( s.startsWith( QLatin1String( "QSize(" ) ) )
	    arg << QVariant( mkSize( s.mid(6, s.length()-7) ) );
	else if ( s.startsWith( QLatin1String( "QRect(" ) ) )
	    arg << QVariant( mkRect( s.mid(6, s.length()-7) ) );
	else if ( s.startsWith( QLatin1String( "QColor(" ) ) )
	    arg << QVariant( mkColor( s.mid(7, s.length()-8) ) );
	else
	    arg << QVariant( s );
    } else if ( type.startsWith("QList<") ||
	        type == "KUrl::List" ) {
	if ( type == "KUrl::List" )
            type = "KUrl";
        else
	    type = type.mid(11, type.length() - 12);
	QStringList list;
	QString delim = s;
	if (delim == "[")
	   delim = "]";
	if (delim == "(")
	   delim = ")";
	i++;
	QByteArray dummy_data;
	QDataStream dummy_arg(&dummy_data, QIODevice::WriteOnly);

	int j = i;
	uint count = 0;
	// Parse list to get the count
	while (true) {
	    if( j >= args.count() )
	    {
		qWarning("List end-delimiter '%s' not found.", delim.toLatin1().constData());
		exit(1);
	    }
	    if( QString::fromLocal8Bit( args[ j ] ) == delim )
		break;
	    marshall( dummy_arg, args, j, type );
	    count++;
	}
	arg << (quint32) count;
	// Parse the list for real
	while (true) {
	    if( i > args.count() )
	    {
		qWarning("List end-delimiter '%s' not found.", delim.toLatin1().constData());
		exit(1);
	    }
	    if( QString::fromLocal8Bit( args[ i ] ) == delim )
		break;
	    marshall( arg, args, i, type );
	}
    } else {
	qWarning( "cannot handle datatype '%s'", type.toLatin1().constData() );
	exit(1);
    }
    i++;
}

// vim: set noet ts=8 sts=4 sw=4:


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

#include <qvariant.h>
#include <qcolor.h>
#include "../../kdecore/kdatastream.h"
#include "../dcopclient.h"
#include "../dcopref.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

static DCOPClient* dcop = 0;

void queryApplications()
{
    QCStringList apps = dcop->registeredApplications();
    for ( QCStringList::Iterator it = apps.begin(); it != apps.end(); ++it )
	if ( (*it) != dcop->appId() && (*it).left(9) != "anonymous" )
	    printf( "%s\n", (*it).data() );

    if ( !dcop->isAttached() )
    {
	qWarning( "server not accessible" );
        exit(1);
    }
}

void queryObjects( const char* app )
{
    bool ok = false;
    QCStringList objs = dcop->remoteObjects( app, &ok );
    for ( QCStringList::Iterator it = objs.begin(); it != objs.end(); ++it ) {
	if ( (*it) == "default" && ++it != objs.end() )
	    printf( "%s (default)\n", (*it).data() );
	else
	    printf( "%s\n", (*it).data() );
    }
    if ( !ok )
    {
	qWarning( "application '%s' not accessible", app );
	exit(1);
    }
}

void queryFunctions( const char* app, const char* obj )
{
    bool ok = false;
    QCStringList funcs = dcop->remoteFunctions( app, obj, &ok );
    for ( QCStringList::Iterator it = funcs.begin(); it != funcs.end(); ++it ) {
	printf( "%s\n", (*it).data() );
    }
    if ( !ok )
    {
	qWarning( "object '%s' in application '%s' not accessible", obj, app );
        exit(1);
    }
}


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

void callFunction( const char* app, const char* obj, const char* func, int argc, char** args )
{

    QString f = func; // Qt is better with unicode strings, so use one.
    int left = f.find( '(' );
    int right = f.find( ')' );

    if ( right <  left )
    {
	qWarning( "parentheses do not match" );
        exit(1);
    }

    if ( left < 0 ) {
	// try to get the interface from the server
	bool ok = false;
	QCStringList funcs = dcop->remoteFunctions( app, obj, &ok );
	QCString realfunc;
	if ( !ok && argc == 0 )
	    goto doit;
	if ( !ok )
        {
	    qWarning( "object not accessible" );
            exit(1);
        }
	for ( QCStringList::Iterator it = funcs.begin(); it != funcs.end(); ++it ) {
	    int l = (*it).find( '(' );
	    int s = (*it).find( ' ');
	    if ( s < 0 )
		s = 0;
	    else
		s++;
	
	    if ( l > 0 && (*it).mid( s, l - s ) == func ) {
		realfunc = (*it).mid( s );
		int a = (*it).contains(',');
		if ( ( a == 0 && argc == 0) || ( a > 0 && a + 1 == argc ) )
		    break;
	    }
	}
	if ( realfunc.isEmpty() )
	{
	    qWarning("no such function");
            exit(1);
	}
	f = realfunc;
	left = f.find( '(' );
	right = f.find( ')' );
    }

 doit:
    if ( left < 0 )
	f += "()";

    QStringList types;
    if ( left >0 && left + 1 < right - 1) {
	types = QStringList::split( ',', f.mid( left + 1, right - left - 1) );
	for ( QStringList::Iterator it = types.begin(); it != types.end(); ++it ) {
	    (*it).stripWhiteSpace();
	    int s = (*it).find(' ');
	    if ( s > 0 )
		(*it) = (*it).left( s );
	}
	QString fc = f.left( left );
	fc += '(';
	bool first = TRUE;
	for ( QStringList::Iterator it = types.begin(); it != types.end(); ++it ) {
	    if ( !first )
		fc +=",";
	    first = FALSE;
	    fc += *it;
	}
	fc += ')';
	f = fc;
    }

    if ( (int) types.count() != argc ) {
	qWarning( "arguments do not match" );
	exit(1);
    }

    QByteArray data, replyData;
    QCString replyType;
    QDataStream arg(data, IO_WriteOnly);

    int i = 0;
    for ( QStringList::Iterator it = types.begin(); it != types.end(); ++it ) {
	QString type = *it;
	QString s = args[i++];
	if ( type == "int" )
	    arg << s.toInt();
	if ( type == "uint" )
	    arg << s.toUInt();
	else if ( type == "long" )
	    arg << s.toLong();
	else if ( type == "float" )
	    arg << s.toFloat();
	else if ( type == "double" )
	    arg << s.toDouble();
	else if ( type == "bool" )
	    arg << mkBool( s );
	else if ( type == "QString" )
	    arg << s;
	else if ( type == "QCString" )
	    arg << QCString( s.latin1() );
	else if ( type == "QColor" )
	    arg << mkColor( s );
	else if ( type == "QPoint" )
	    arg << mkPoint( s );
	else if ( type == "QSize" )
	    arg << mkSize( s );
	else if ( type == "QRect" )
	    arg << mkRect( s );
	else if ( type == "QVariant" ) {
	    if ( s == "true" || s == "false" )
		arg << QVariant( mkBool( s ), 42 );
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
	} else {
	    qWarning( "cannot handle datatype '%s'", type.latin1() );
	    exit(1);
	}
    }

    if ( !dcop->call( app, obj, f.latin1(),  data, replyType, replyData) ) {
	qWarning( "call failed");
        exit(1);
    } else {
	QDataStream reply(replyData, IO_ReadOnly);
	if ( replyType == "int" ) {
	    int i;
	    reply >> i;
	    printf( "%d\n", i );
	} else if ( replyType == "uint" ) {
	    uint i;	
	    reply >> i;
	    printf( "%d\n", i );
	} else if ( replyType == "long" ) {
	    long l;
	    reply >> l;
	    printf( "%ld\n", l );
	} else if ( replyType == "long" ) {
	    long l;
	    reply >> l;
	    printf( "%ld\n", l );
	} else if ( replyType == "float" ) {
	    float f;
	    reply >> f;
	    printf( "%f\n", (double) f );
	} else if ( replyType == "double" ) {
	    double d;
	    reply >> d;
	    printf( "%f\n", d );
	} else if (replyType == "bool") {
	    bool b;
	    reply >> b;
	    printf( "%s\n", b ? "true" : "false" );
	} else if (replyType == "QString") {
	    QString r;
	    reply >> r;
	    printf( "%s\n", r.latin1() );
	} else if (replyType == "QCString") {
	    QCString r;
	    reply >> r;
	    printf( "%s\n", r.data() );
	} else if (replyType == "QCStringList") {
	    QCStringList l;
	    reply >> l;
	    for ( QCStringList::Iterator it = l.begin(); it != l.end(); ++it )
		printf( "%s\n", (*it).data() );
	} else if (replyType == "QStringList") {
	    QStringList l;
	    reply >> l;
	    for ( QStringList::Iterator it = l.begin(); it != l.end(); ++it )
		printf( "%s\n", (*it).latin1() );
	} else if (replyType == "QColor") {
	    QColor c;
	    reply >> c;
	    printf( "%s\n", c.name().latin1() );
	} else if (replyType == "QSize") {
	    QSize s;
	    reply >> s;
	    printf( "%dx%d\n", s.width(), s.height() );
	} else if (replyType == "QPoint") {
	    QPoint p;
	    reply >> p;
	    printf( "+%d+%d\n", p.x(), p.y() );
	} else if (replyType == "QRect") {
	    QRect r;
	    reply >> r;
	    printf( "%dx%d+%d+%d\n", r.width(), r.height(), r.x(), r.y() );
	} else if (replyType == "QVariant") {
	    QVariant v;
	    reply >> v;
	    if ( v.type() == QVariant::Bool )
		printf( "%s\n", v.toBool() ? "true" : "false" );
	    else if ( v.type() == QVariant::Rect ) {
		QRect r = v.toRect();
		printf( "QRect(%d,%d,%d,%d)\n", r.x(), r.y(), r.width(), r.height() );
	    } else if ( v.type() == QVariant::Point ) {
		QPoint p = v.toPoint();
		printf( "QPoint(%d,%d)\n", p.x(), p.y() );
	    } else if ( v.type() == QVariant::Size ) {
		QSize s = v.toSize();
		printf( "QSize(%d,%d)\n", s.width(), s.height() );
	    } else if ( v.type() == QVariant::Color ) {
		QColor s = v.toColor();
		printf( "QColor(%s)\n", s.name().latin1() );
	    } else if ( v.canCast( QVariant::Int ) )
		printf( "%d\n", v.toInt() );
	    else if ( !v.toString().isNull() )
		printf( "%s\n", v.toString().latin1() );
	    else
		printf( "<%s>\n", QVariant::typeToName( v.type() ) );
	} else if ( replyType == "DCOPRef" ) {
	    DCOPRef r;
	    reply >> r;
	    printf( "DCOPRef(%s,%s)\n", r.app().data(), r.object().data() );
	} else if ( !replyType.isEmpty() && replyType != "void" && replyType != "ASYNC" ) {
	    printf( "<%s>\n",  replyType.data() );
	}
    }
}



int main( int argc, char** argv )
{
    
    if ( argc > 1 && argv[1][0] == '-' ) {
	fprintf( stderr, "Usage: dcop [ application [object [function [arg1] [arg2] [arg3] ... ] ] ] \n" );
	exit(0);
    }
    
    DCOPClient client;
    client.attach();
    dcop = &client;
    
    switch ( argc ) {
    case 0:
    case 1:
	queryApplications();
	break;
    case 2:
	queryObjects( argv[1] );
	break;
    case 3:
	queryFunctions( argv[1], argv[2] );
	break;
    case 4:
    default:
	callFunction( argv[1], argv[2], argv[3], argc - 4, &argv[4] );
	break;

    }

    return 0;
}

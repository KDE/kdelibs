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

#include "../../kdecore/kdatastream.h"
#include "../../kdecore/kdatastream.cpp"
#include "../dcopclient.h"
#include <stdlib.h>
#include <stdio.h>

static DCOPClient* dcop = 0;

void queryApplications()
{
    QCStringList apps = dcop->registeredApplications();
    for ( QCStringList::Iterator it = apps.begin(); it != apps.end(); ++it )
    if ( (*it) != dcop->appId() && (*it).left(9) != "anonymous" )
        fprintf( stdout, "%s\n", (*it).data() );

    if ( !dcop->isAttached() )
    qFatal( "server not accessible" );
}

void queryObjects( const char* app )
{
    bool ok = false;
    QCStringList objs = dcop->remoteObjects( app, &ok );
    for ( QCStringList::Iterator it = objs.begin(); it != objs.end(); ++it ) {
    if ( (*it) == "default" && ++it != objs.end() )
        fprintf( stdout, "%s (default)\n", (*it).data() );
    else
        fprintf( stdout, "%s\n", (*it).data() );
    }
    if ( !ok )
    qFatal( "application not accessible" );
}

void queryFunctions( const char* app, const char* obj )
{
    bool ok = false;
    QCStringList funcs = dcop->remoteFunctions( app, obj, &ok );
    for ( QCStringList::Iterator it = funcs.begin(); it != funcs.end(); ++it ) {
    if ( (*it) == "QCStringList functions()" )
        continue;
    fprintf( stdout, "%s\n", (*it).data() );
    }
    if ( !ok )
     qFatal( "object not accessible" );
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

void callFunction( const char* app, const char* obj, const char* func, int argc, char** args )
{

    QString f = func; // Qt is better with unicode strings, so use one.
    int left = f.find( '(' );
    int right = f.find( ')' );

    if ( right <  left )
    qFatal( "parentheses do not match" );

    if ( left < 0 ) {
    // try to get the interface from the server
    bool ok = false;
    QCStringList funcs = dcop->remoteFunctions( app, obj, &ok );
    QCString realfunc;
    if ( !ok )
        qFatal( "object not accessible" );
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
        if ( a + argc == 0 || a + 1 == argc )
            break;
        }
    }
    if ( realfunc.isEmpty() )
        qFatal("no such function");
    f = realfunc;
    left = f.find( '(' );
    right = f.find( ')' );
    }

    QStringList types;
    if ( left >0 && left + 1 < right - 1) {
    types = QStringList::split( ',', f.mid( left + 1, right - left - 1) );
    for ( QStringList::Iterator it = types.begin(); it != types.end(); ++it )
        (*it).stripWhiteSpace();
    }

    if ( (int) types.count() != argc ) {
    qFatal( "arguments do not match" );
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
    else
        qFatal( "cannot handle datatype '%s'", type.latin1() );
    }

    if ( !dcop->call( app, obj, f.latin1(),  data, replyType, replyData) ) {
    qFatal( "call failed");
    } else {
    QDataStream reply(replyData, IO_ReadOnly);
    if ( replyType == "int" ) {
        int i;
        reply >> i;
        fprintf( stdout, "%d\n", i );
    } else if ( replyType == "long" ) {
        long l;
        reply >> l;
        fprintf( stdout, "%ld\n", l );
    } else if ( replyType == "long" ) {
        long l;
        reply >> l;
        fprintf( stdout, "%ld\n", l );
    } else if ( replyType == "float" ) {
        float f;
        reply >> f;
        fprintf( stdout, "%f\n", (double) f );
    } else if ( replyType == "double" ) {
        double d;
        reply >> d;
        fprintf( stdout, "%f\n", d );
    } else if (replyType == "bool") {
        bool b;
        reply >> b;
        fprintf( stdout, "%s\n", b ? "true" : "false" );
    } else if (replyType == "QString") {
        QString r;
        reply >> r;
        fprintf( stdout, "%s\n", r.latin1() );
    } else if (replyType == "QCString") {
        QCString r;
        reply >> r;
        fprintf( stdout, "%s\n", r.data() );
    }
    }
}



int main( int argc, char** argv )
{
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

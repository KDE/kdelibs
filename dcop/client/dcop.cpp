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
#include "../kdatastream.h"
#include "../dcopclient.h"
#include "../dcopref.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "marshall.cpp"

static DCOPClient* dcop = 0;

bool startsWith(const QCString &id, const char *str, int n)
{
  return !n || (strncmp(id.data(), str, n) == 0);
}

bool endsWith(QCString &id, char c)
{
   if (id.length() && (id[id.length()-1] == c))
   {
      id.truncate(id.length()-1);
      return true;
   }
   return false;
}

void queryApplications(const QCString &filter)
{
    int filterLen = filter.length();        
    QCStringList apps = dcop->registeredApplications();
    for ( QCStringList::Iterator it = apps.begin(); it != apps.end(); ++it )
    {
        QCString &clientId = *it;
	if ( (clientId != dcop->appId()) && 
             !startsWith(clientId, "anonymous",9) &&
             startsWith(clientId, filter, filterLen)
           )
	    printf( "%s\n", clientId.data() );
    }

    if ( !dcop->isAttached() )
    {
	qWarning( "server not accessible" );
        exit(1);
    }
}

void queryObjects( const QCString &app, const QCString &filter )
{
    int filterLen = filter.length();        
    bool ok = false;
    bool isDefault = false;
    QCStringList objs = dcop->remoteObjects( app, &ok );
    for ( QCStringList::Iterator it = objs.begin(); it != objs.end(); ++it ) 
    {
        QCString &objId = *it;

        if (objId == "default")
        {
           isDefault = true;
           continue;
        }

        if (startsWith(objId, filter, filterLen))
        {
            if (isDefault)
                printf( "%s (default)\n", objId.data() );
            else
                printf( "%s\n", objId.data() );
        }
        isDefault = false;
    }
    if ( !ok )
    {
        if (!dcop->isApplicationRegistered(app))
            qWarning( "No such application: '%s'", app.data());
        else
            qWarning( "Application '%s' not accessible", app.data() );
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

    // This may seem expensive but is done only once per invocation
    // of dcop, so it should be OK.
    //
    //
    QStringList intTypes;
    intTypes << "int" << "unsigned" << "long" << "bool" ;

    QStringList types;
    if ( left >0 && left + 1 < right - 1) {
	types = QStringList::split( ',', f.mid( left + 1, right - left - 1) );
	for ( QStringList::Iterator it = types.begin(); it != types.end(); ++it ) {
	    QString lt = (*it).simplifyWhiteSpace();

	    int s = lt.find(' ');

	    // If there are spaces in the name, there may be two
	    // reasons: the parameter name is still there, ie.
	    // "QString URL" or it's a complicated int type, ie.
	    // "unsigned long long int bool".
	    //
	    //
	    if ( s > 0 )
	    {
		QStringList partl = QStringList::split(' ' , lt);
		
		// The zero'th part is -- at the very least -- a 
		// type part. Any trailing parts *might* be extra
		// int-type keywords, or at most one may be the
		// parameter name.
		//
		//
	    	s=1;

		while (s < (int)partl.count() && intTypes.contains(partl[s]))
		{
			s++;
		}

		if (s<(int)partl.count()-1)
		{
			qWarning("The argument `%s' seems syntactically wrong.",
				lt.latin1());
		}
		if (s==(int)partl.count()-1)
		{
			partl.remove(partl.at(s));
		}

	    	lt = partl.join(" ");
		lt = lt.simplifyWhiteSpace();
	    }

	    (*it) = lt;
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
        marshall(arg, args[i++], *it);
    }

    if ( !dcop->call( app, obj, f.latin1(),  data, replyType, replyData) ) {
	qWarning( "call failed");
        exit(1);
    } else {
	QDataStream reply(replyData, IO_ReadOnly);

        if ( replyType != "void" && replyType != "ASYNC" )
        {
            QCString replyString = demarshal( reply, replyType );
            if ( replyString.isEmpty() )
                replyString.sprintf( "<%s>", replyType.data() );

            printf( "%s\n", replyString.data() );
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

    QCString app;
    QCString objid;
    QCString function;
    char **args = 0;
    if ((argc > 1) && (strncmp(argv[1], "DCOPRef(", 8)) == 0)
    {
       char *delim = strchr(argv[1], ',');
       if (!delim)
       {
          fprintf(stderr, "Error: '%s' is not a valid DCOP reference.\n", argv[1]);
          return 1;
       }
       *delim = 0;
       app = argv[1] + 8;
       delim++;
       delim[strlen(delim)-1] = 0;
       objid = delim;
       if (argc > 2)
          function = argv[2];
       if (argc > 3)
          args = &argv[3];
       argc++;
    }
    else
    {
       if (argc > 1)
          app = argv[1];
       if (argc > 2)
          objid = argv[2];
       if (argc > 3)
          function = argv[3];
       if (argc > 4)
          args = &argv[4];
    }

    switch ( argc ) {
    case 0:
    case 1:
	queryApplications("");
	break;
    case 2:
        if (endsWith(app, '*'))
           queryApplications(app);
        else
           queryObjects( app, "" );
	break;
    case 3:
        if (endsWith(objid, '*'))
           queryObjects(app, objid);
        else
           queryFunctions( app, objid );
	break;
    case 4:
    default:
	callFunction( app, objid, function, argc - 4, args );
	break;

    }

    return 0;
}

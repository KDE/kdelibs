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

#include "../dcopclient.h"
#include <stdlib.h>
#include <stdio.h>

static DCOPClient* dcop = 0;

bool queryApplications()
{
    QCStringList apps = dcop->registeredApplications();
    for ( QCStringList::Iterator it = apps.begin(); it != apps.end(); ++it )
	if ( (*it) != dcop->appId() && (*it).left(9) != "anonymous" )
	    fprintf( stdout, "%s\n", (*it).data() );

    return dcop->isAttached();
}

bool queryObjects( const char* app )
{
    bool ok = false;
    QCStringList objs = dcop->remoteObjects( app, &ok );
    for ( QCStringList::Iterator it = objs.begin(); it != objs.end(); ++it ) {
	if ( (*it) == "default" && ++it != objs.end() )
	    fprintf( stdout, "%s (default)\n", (*it).data() );
	else
	    fprintf( stdout, "%s\n", (*it).data() );
    }

    return ok;
}

bool queryFunctions( const char* app, const char* obj )
{
    bool ok = false;
    QCStringList funcs = dcop->remoteFunctions( app, obj, &ok );
    for ( QCStringList::Iterator it = funcs.begin(); it != funcs.end(); ++it ) {
	if ( (*it) == "QCStringList functions()" )
	    continue;
	fprintf( stdout, "%s\n", (*it).data() );
    }

    return ok;
}

bool callFunction( const char* app, const char* obj, const char* func, int argc, char** args )
{
    bool ok = false;


    return ok;
}



int main( int argc, char** argv )
{
    bool ok = false;

    DCOPClient client;
    client.attach();
    dcop = &client;

    switch ( argc ) {
    case 0:
    case 1:
	ok = queryApplications();
	break;
    case 2:
	ok = queryObjects( argv[1] );
	break;
    case 3:
	ok = queryFunctions( argv[1], argv[2] );
	break;
    case 4:
	ok = callFunction( argv[1], argv[2], argv[3], argc - 4, &argv[4] );
	break;
	
    }

    return ok ? 0: 1 ;
}

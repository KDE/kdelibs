/*****************************************************************
Copyright (c) 1999 Torben Weis <weis@kde.org>
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
#include <qdom.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qstring.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "main.h"



void usage()
{
    fprintf( stderr, "Usage: dcopidl2cpp [ --no-skel | --no-stub ] [--c++-suffix <suffix>] file\n" );
}

int main( int argc, char** argv )
{

    if ( *qVersion() == '1' ) {
        fprintf( stderr, "dcopidl2cpp appears to be linked to Qt 1 instead of Qt 2 ! Aborting.\n" );
        exit(1);
    }
    int argpos = 1;
    bool generate_skel = TRUE;
    bool generate_stub = TRUE;

    QString suffix = "cpp";

    while (argc > 2) {

	if ( strcmp( argv[argpos], "--no-skel" ) == 0 )
	{
	    generate_skel = FALSE;
	    for (int i = argpos; i < argc - 1; i++) argv[i] = argv[i+1];
	    argc--;
	}
	else if ( strcmp( argv[argpos], "--no-stub" ) == 0 )
	{
	    generate_stub = FALSE;
	    for (int i = argpos; i < argc - 1; i++) argv[i] = argv[i+1];
	    argc--;
	}
	else if ( strcmp( argv[argpos], "--c++-suffix" ) == 0)
	{
	    if (argc - 1 < argpos) {
		usage();
		exit(1);
	    }
	    suffix = argv[argpos+1];
	    for (int i = argpos; i < argc - 2; i++) argv[i] = argv[i+2];
	    argc -= 2;
	} else {
	    usage();
	    exit(1);

	}
    }

    QFile in( argv[argpos] );
    if ( !in.open( IO_ReadOnly ) )
	qFatal("Could not read %s", argv[argpos] );

    QDomDocument doc;
    doc.setContent( &in );

    QDomElement de = doc.documentElement();
    Q_ASSERT( de.tagName() == "DCOP-IDL" );

    QString base( argv[argpos] );
    QString idl = base;

    int pos = base.findRev( '.' );
    if ( pos != -1 )
	base = base.left( pos );

    pos = idl.findRev('/');
    if ( pos != -1 )
	idl = idl.mid( pos+1 );

    if ( generate_skel )
	generateSkel( idl, base + "_skel." + suffix, de );

    if ( generate_stub ) {
	QString header = base + "_stub.h";
	generateStub( idl, header, de );
	pos = header.findRev('/');
	if ( pos != -1 )
	    header = header.mid( pos+1 );
	generateStubImpl( idl, header, base + "_stub." + suffix, de );
    }

    return 0;
}

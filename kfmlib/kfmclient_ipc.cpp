/* This file is part of the KDE libraries
    Copyright (C) 1997 Torben Weis (weis@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
// This file has been created by ipcc.pl.
// (c) Torben Weis
//     weis@stud.uni-frankfurt.de

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "kfmipc.h"
#include "kfmclient_ipc.h"

KfmIpc::KfmIpc( char * _path )
{
    bHeader = TRUE;
    cHeader = 0;
    pBody = 0L;

    sock = new KSocket( _path );
    connect( sock, SIGNAL( readEvent(KSocket*) ), this, SLOT( readEvent(KSocket*) ) );
    connect( sock, SIGNAL( closeEvent(KSocket*) ), this, SLOT( closeEvent(KSocket*) ) );
    sock->enableRead( TRUE );
    connected = TRUE;
}

KfmIpc::~KfmIpc()
{
    delete sock;
    if (pBody != 0)
      free(pBody);
}

bool KfmIpc::isConnected()
{
    return connected;
}

void KfmIpc::closeEvent( KSocket * )
{
    connected = FALSE;
}

void KfmIpc::readEvent( KSocket * )
{
    if ( bHeader )
    {
	int n;
	n = read( sock->socket(), headerBuffer + cHeader, 1 );
	if ( headerBuffer[ cHeader ] == ' ' )
	{
	    bHeader = FALSE;
	    cHeader = 0;
	    bodyLen = atoi( headerBuffer );
	    cBody = 0;
	    if ( bodyLen <= 0 )
	    {
		printf("ERROR: Invalid header\n");
		delete this;
		return;
	    }
	    if ( pBody != 0L )
		free( pBody );
	    pBody = (char*)malloc( bodyLen + 1 );
	}
	else if ( cHeader + n == 10 )
	{
	    printf("ERROR: Too long header\n");
	    delete this;
	    return;
	}
	else
	{
	    if ( !isdigit( headerBuffer[ cHeader ] ) )
	    {
		printf("ERROR: Header must be an int\n");
		delete this;
		return;
	    }

	    cHeader += n;
	    return;
	}
    }
	
    int n;
    n = read( sock->socket(), pBody + cBody, bodyLen - cBody );
    if ( n + cBody == bodyLen )
    {
	pBody[bodyLen] = 0;
	bHeader = TRUE;
	parse( pBody, bodyLen );
	return;
    }
    cBody += n;
}

void KfmIpc::parse( char *_data, int _len )
{
    int pos = 0;
    char *name = read_string( _data, pos, _len );
    if ( name == 0L )
	return;
    _data += pos;
    _len -= pos;

    if ( strcmp( name, "finished" ) == 0 ) 
      { parse_finished( _data, _len ); } 
    else
      if ( strcmp( name, "error" ) == 0 ) 
	{ parse_error( _data, _len ); } 
      else
	if ( strcmp( name, "dirEntry" ) == 0 ) 
	  { parse_dirEntry( _data, _len ); } 
	else
	  { printf("Unknown command '%s'\n",name); }
    free(name);
}


#include "kfmclient_ipc.moc"

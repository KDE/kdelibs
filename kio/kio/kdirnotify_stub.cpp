/****************************************************************************
**
** DCOP Stub Implementation based on output of dcopidl2cpp from kdirnotify.kidl
** but with hand coded changes!!
**
*****************************************************************************/
/* This file is part of the KDE project
   Copyright (C) 2000 David Faure <faure@kde.org>
   Copyright (C) 2003 Waldo Bastian <bastian@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kdirnotify_stub.h"
#include <dcopclient.h>

#include <kdatastream.h>


KDirNotify_stub::KDirNotify_stub( const DCOPCString& app, const DCOPCString& obj )
  : DCOPStub( app, obj )
{
}

KDirNotify_stub::KDirNotify_stub( DCOPClient* client, const DCOPCString& app, const DCOPCString& obj )
  : DCOPStub( client, app, obj )
{
}

KDirNotify_stub::KDirNotify_stub( const DCOPRef& ref )
  : DCOPStub( ref )
{
}

void KDirNotify_stub::FilesAdded( const KURL& arg0 )
{
    if ( !dcopClient()  ) {
	setStatus( CallFailed );
	return;
    }
    QByteArray data;
    QDataStream arg( &data, QIODevice::WriteOnly );
    arg << arg0;
    dcopClient()->emitDCOPSignal( "KDirNotify", "FilesAdded(KURL)", data );
    setStatus( CallSucceeded );
}

void KDirNotify_stub::FilesRemoved( const KURL::List& arg0 )
{
    if ( !dcopClient()  ) {
	setStatus( CallFailed );
	return;
    }
    QByteArray data;
    QDataStream arg( &data, QIODevice::WriteOnly );
    arg << arg0;
    dcopClient()->emitDCOPSignal( "KDirNotify", "FilesRemoved(KURL::List)", data );
    setStatus( CallSucceeded );
}

void KDirNotify_stub::FilesChanged( const KURL::List& arg0 )
{
    if ( !dcopClient()  ) {
	setStatus( CallFailed );
	return;
    }
    QByteArray data;
    QDataStream arg( &data, QIODevice::WriteOnly );
    arg << arg0;
    dcopClient()->emitDCOPSignal( "KDirNotify", "FilesChanged(KURL::List)", data );
    setStatus( CallSucceeded );
}

void KDirNotify_stub::FileRenamed( const KURL& arg0, const KURL& arg1 )
{
    if ( !dcopClient()  ) {
	setStatus( CallFailed );
	return;
    }
    QByteArray data;
    QDataStream arg( &data, QIODevice::WriteOnly );
    arg << arg0;
    arg << arg1;
    dcopClient()->emitDCOPSignal( "KDirNotify", "FileRenamed(KURL,KURL)", data );
    setStatus( CallSucceeded );
}



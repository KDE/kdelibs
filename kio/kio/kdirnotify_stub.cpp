/****************************************************************************
**
** DCOP Stub Implementation based on output of dcopidl2cpp from kdirnotify.kidl
** but with hand coded changes!!
**
*****************************************************************************/

#include "kdirnotify_stub.h"
#include <dcopclient.h>

#include <kdatastream.h>


KDirNotify_stub::KDirNotify_stub( const QCString& app, const QCString& obj )
  : DCOPStub( app, obj )
{
}

KDirNotify_stub::KDirNotify_stub( DCOPClient* client, const QCString& app, const QCString& obj )
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
    QDataStream arg( data, IO_WriteOnly );
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
    QDataStream arg( data, IO_WriteOnly );
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
    QDataStream arg( data, IO_WriteOnly );
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
    QDataStream arg( data, IO_WriteOnly );
    arg << arg0;
    arg << arg1;
    dcopClient()->emitDCOPSignal( "KDirNotify", "FileRenamed(KURL,KURL)", data );
    setStatus( CallSucceeded );
}



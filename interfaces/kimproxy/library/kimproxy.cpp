/*    
	kimproxy.cpp
	
	IM service library for KDE
	
	Copyright (c) 2004 Will Stephenson   <lists@stevello.free-online.co.uk>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <dcopclient.h>
#include <kdcopservicestarter.h> 
#include <kdebug.h>
#include <kmessagebox.h>
#include <ksimpleconfig.h>
#include <kiconloader.h>

#include "kimiface_stub.h"

#include "kimproxy.h"

KIMProxy::KIMProxy( DCOPClient* dc ) : DCOPObject( "KIMProxyIface" ), QObject()
{
	m_im_client_stub = 0L;
	m_dc = dc;
	// hope the notifications is refcounted!
	m_dc->setNotifications( true );
	connect( m_dc, SIGNAL( applicationRemoved( const QCString& ) ) , this, SLOT( unregisteredFromDCOP( const QCString& ) ) );
	//QCString senderApp = "Kopete";
	//QCString senderObjectId = "KIMIface";
	QCString method = "contactStatusChanged(QString)";
	//QCString receiverObjectId = "KIMProxyIface";
	
	// FIXME: make this work when the sender object id is set to KIMIFace
	if ( !connectDCOPSignal( 0, 0, method, method, false ) )
		KMessageBox::information( 0, QString( "Couldn't connect DCOP signal.\nWon't receive any status notifications!" ) );
}

KIMProxy::~KIMProxy( )
{
	m_dc->setNotifications( false );
	delete m_im_client_stub;
}

bool KIMProxy::initialize()
{
	if ( !m_im_client_stub )
	{
		// start/find an instance of DCOP/InstantMessenger
		QString error;
		QCString dcopService;
		// Get a preferred IM client
		QString preferences = QString::null;
		// FIXME: we are getting preferences from a config specified by hardcoded literals - the interface or kcm_componentchooser specification should be all we need to know and say where we get preferences from.
		KConfig *store = new KSimpleConfig( IM_CLIENT_PREFERENCES_FILE );
		store->setGroup( IM_CLIENT_PREFERENCES_SECTION );
		QString preferredApp = store->readEntry( IM_CLIENT_PREFERENCES_ENTRY );
		kdDebug() << k_funcinfo << "found preferred app: " << preferredApp << endl;
		if ( !preferredApp.isNull() )
		{
			// construct a preferences trader expression - Name == value
			preferences = QString("[X-DCOP-ServiceName] == '%1'").arg( preferredApp );
		}	
		// FIXME: we never get any hits if searching using the obvious kinds of prefs expressions - maybe they do not fit 'an expression in the constraint language that must return a number' (ktrader.h)
		//int result = KDCOPServiceStarter::self()->findServiceFor( IM_SERVICE_TYPE, QString::null, QString::null, &error, &dcopService );
		int result = KDCOPServiceStarter::self()->findServiceFor( IM_SERVICE_TYPE, QString::null, preferences, &error, &dcopService );
		
		// set up our stub, connecting to the client
		if ( result != 0 )
		{
			return false; // FIXME return error
			KMessageBox::error( 0, QString( "Couldn't find an IM service.\nCheck you have one selected in KControl ->Component Chooser\ndebug error: %1, using query: %2" ).arg( error ).arg( preferences ) );
		}
		QCString dcopObjectId = "KIMIface";
		m_im_client_stub = new KIMIface_stub( m_dc, dcopService, dcopObjectId );
	}
	return m_im_client_stub != 0L;
}

QStringList KIMProxy::imAddresseeUids()
{
	QStringList value;
	
	if ( initialize() )
	{
		value = m_im_client_stub->allContacts( );
	}
	return value;
}

void KIMProxy::messageContactById( const QString& uid, const QString& message )
{
	if ( initialize() )
	{
		m_im_client_stub->messageContact( uid, message );
	}
	return;
}

void KIMProxy::chatContactById( const QString& uid )
{
	if ( initialize() )
	{
		m_im_client_stub->chatWithContact( uid );
	}
	return;
}

void KIMProxy::sendFileToId(const QString &metaContactId, const KURL &sourceURL, const QString &altFileName, uint fileSize )
{
	if ( initialize() )
	{
		m_im_client_stub->sendFile( metaContactId, sourceURL, altFileName, fileSize );
	}
	return;
}

int KIMProxy::statusNumeric( const QString& uid )
{
	if ( initialize() )
	{
		// get a QString from  m_kim_client_stub->onlineStatus( uid );
		// and turn it into an OnlineStatus enum
		return m_im_client_stub->presenceStatus( uid );
	}
	return -1;
}

QString KIMProxy::statusString( const QString& uid )
{
	if ( initialize() )
	{
		// get a QString from  m_kim_client_stub->onlineStatus( uid );
		// and turn it into an OnlineStatus enum
		return m_im_client_stub->presenceString( uid );
	}
	return QString("NONE");
}

QPixmap KIMProxy::statusIcon( const QString& uid )
{
	if ( initialize() )
	{
		return m_im_client_stub->icon( uid );
	}
	return QPixmap();
}
	
void KIMProxy::unregisteredFromDCOP( const QCString& appId )
{
	if ( m_im_client_stub && m_im_client_stub->app() == appId )
	{
		delete m_im_client_stub;
		m_im_client_stub = 0L;
	}
}

void KIMProxy::contactStatusChanged( QString uid )
{
	emit sigContactStatusChanged( uid );
}

#include "kimproxy.moc"

/*    
	kimproxy.cpp
	
	IM service library for KDE
	
	Copyright (c) 2004 Will Stephenson   <lists@stevello.free-online.co.uk>

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

#include <qpixmapcache.h>
#include <dcopclient.h>
#include <kdcopservicestarter.h> 
#include <kdebug.h>
#include <kmessagebox.h>
#include <ksimpleconfig.h>
#include <kiconloader.h>
#include <kservice.h>
#include <kservicetype.h>

#include "kimiface_stub.h"

#include "kimproxy.h"

static KStaticDeleter<KIMProxy> _staticDeleter;

KIMProxy * KIMProxy::s_instance = 0L;

KIMProxy * KIMProxy::instance( DCOPClient * client ) 
{
	if ( client )
	{
		if ( !s_instance )
			_staticDeleter.setObject( s_instance, new KIMProxy( client ) );
		return s_instance;
	}
	else
		return 0L;
}

KIMProxy::KIMProxy( DCOPClient* dc ) : DCOPObject( "KIMProxyIface" ), QObject()
{
	m_im_client_stubs.setAutoDelete( true );
	m_apps_available = false;
	m_dc = dc;
	m_initialized = false;
	connect( m_dc, SIGNAL( applicationRemoved( const QCString& ) ) , this, SLOT( unregisteredFromDCOP( const QCString& ) ) );
	connect( m_dc, SIGNAL( applicationRegistered( const QCString& ) ) , this, SLOT( registeredToDCOP( const QCString& ) ) );
	m_dc->setNotifications( true );

	//QCString senderApp = "Kopete";
	//QCString senderObjectId = "KIMIface";
	QCString method = "contactPresenceChanged( QString, QCString, int )";
	//QCString receiverObjectId = "KIMProxyIface";
	
	// FIXME: make this work when the sender object id is set to KIMIFace
	if ( !connectDCOPSignal( 0, 0, method, method, false ) )
		KMessageBox::information( 0, QString( "Couldn't connect DCOP signal.\nWon't receive any status notifications!" ) );
}

KIMProxy::~KIMProxy( )
{
	m_dc->setNotifications( false );
}

bool KIMProxy::initialize()
{
	if ( !m_initialized )
	{
		m_initialized = true; // we should only do this once, as registeredToDCOP() will catch any new starts
		// So there is no error from a failed query when using kdelibs 3.2, which don't have this servicetype
		if ( KServiceType::serviceType( IM_SERVICE_TYPE ) ) 
		{
			kdDebug( 5301 ) << k_funcinfo << endl;
			QCString dcopObjectId = "KIMIface";
	
			// see what apps implementing our service type are out there
			KService::List offers = KServiceType::offers( IM_SERVICE_TYPE );
			KService::List::iterator it;
			for ( it = offers.begin(); it != offers.end(); ++it )
			{
				QCString dcopService = (*it)->property("X-DCOP-ServiceName").toString().latin1();
				if ( !dcopService.isEmpty() ) {
					m_apps_available = true;
					kdDebug( 5301 ) << " app name: " << (*it)->name() << " dcopService: " << dcopService << endl;
					if ( m_dc->isApplicationRegistered( dcopService ) && !m_im_client_stubs.find( dcopService ) )
					{
						kdDebug( 5301 ) << "inserting new stub for " << dcopService << " dcopObjectId " << dcopObjectId << endl;
						m_im_client_stubs.insert( dcopService, new KIMIface_stub( m_dc, dcopService, dcopObjectId ) );
					}
				}
			}
		}
	}
	return !m_im_client_stubs.isEmpty();
}

QStringList KIMProxy::allContacts()
{
	QStringList value;
	
	if ( initialize() )
	{
		QDictIterator<KIMIface_stub> it( m_im_client_stubs );
		for ( ; it.current(); ++it )
		{
			value += it.current()->allContacts();
		}
	}
	return value;
}

QStringList KIMProxy::reachableContacts()
{
	QStringList value;
	
	if ( initialize() )
	{
		QDictIterator<KIMIface_stub> it( m_im_client_stubs );
		for ( ; it.current(); ++it )
		{
			value += it.current()->reachableContacts( );
		}
	}
	return value;
}

QStringList KIMProxy::onlineContacts()
{
	QStringList value;
	
	if ( initialize() )
	{
		QDictIterator<KIMIface_stub> it( m_im_client_stubs );
		for ( ; it.current(); ++it )
		{
			value += it.current()->onlineContacts( );
		}
	}
	return value;
}

QStringList KIMProxy::fileTransferContacts()
{
	QStringList value;
	
	if ( initialize() )
	{
		QDictIterator<KIMIface_stub> it( m_im_client_stubs );
		for ( ; it.current(); ++it )
		{
			value += it.current()->fileTransferContacts( );
		}
	}
	return value;
}

bool KIMProxy::isPresent( const QString& uid )
{
	bool present = false;
	if ( initialize() )
	{
		QDictIterator<KIMIface_stub> it( m_im_client_stubs );
		for ( ; it.current(); ++it )
		{
			if (it.current()->isPresent( uid ) )
			{
				present = true;
		 		break;
			}
		}
    }
    return present;
}

QString KIMProxy::displayName( const QString& uid )
{
	QString name;
	if ( initialize() )
	{
		kdDebug( 5301 ) << "initialized OK!"<<endl;
		name = stubForUid( uid )->displayName( uid );
	}
	kdDebug( 5301 ) << k_funcinfo << name << endl;
    return name;
}

int KIMProxy::presenceNumeric( const QString& uid )
{
	int presence = -1;
	if ( initialize() )
	{
		pollAll( uid );
		AppPresence *ap = m_presence_map.find( uid );
		if ( ap ) // find returns 0 on not found.
			presence = ap->presence;
	}
	return presence;
}

QString KIMProxy::presenceString( const QString& uid )
{
    kdDebug( 5301 ) << k_funcinfo << endl;
	
	QString presence;
	if ( initialize() )
	{
		PresenceStringMap * appPresenceStrings = m_client_presence_strings[ stubForUid( uid ) ];
		if ( !appPresenceStrings ) // we have no presence strings mapped at all for this stub
		{
			kdDebug( 5301 ) << " no string cache found for this stub , creating one" << endl;
			appPresenceStrings = new PresenceStringMap();
			m_client_presence_strings.insert( stubForUid( uid ), appPresenceStrings );
		}
		int numeric = presenceNumeric( uid );
		presence = (*appPresenceStrings)[ numeric ];
		if ( presence.isEmpty() ) //  cache miss
		{
			kdDebug( 5301 ) << " no cached string found for this app, fetching it" << endl;
			presence = stubForUid( uid )->presenceString( uid );
			appPresenceStrings->insert( numeric, presence );
		}
		kdDebug( 5301 ) << " resulting presence string for " << uid << " : " << presence << endl;
	}
	return presence;
}

QPixmap KIMProxy::presenceIcon( const QString& uid )
{
	QPixmap presence;
	if ( initialize() )
	{
		// get the app id 
		pollAll( uid );
		AppPresence *existing = m_presence_map.find( uid );
		// try the pixmap cache, and insert if not found
		QString appPresenceKey = existing->appId + presenceNumeric( uid );
		if ( !QPixmapCache::find( appPresenceKey, presence ) )
		{
			presence = stubForUid( uid )->icon( uid );
			QPixmapCache::insert( appPresenceKey, presence );
		}
	}
	return presence;
}

bool KIMProxy::canReceiveFiles( const QString & uid )
{
	if ( initialize() )
	{
		return stubForUid( uid )->canReceiveFiles( uid );
	}
	return false;
}

bool KIMProxy::canRespond( const QString & uid )
{
	if ( initialize() )
	{
		return stubForUid( uid )->canRespond( uid );
	}
	return false;
}

QString KIMProxy::context( const QString & uid )
{
	if ( initialize() )
	{
		return stubForUid( uid )->context( uid );
	}
	return QString::null;
}
	
void KIMProxy::chatWithContact( const QString& uid )
{
	if ( initialize() )
	{
		stubForUid( uid )->chatWithContact( uid );
	}
	return;
}

void KIMProxy::messageContact( const QString& uid, const QString& message )
{
	if ( initialize() )
	{
		stubForUid( uid )->messageContact( uid, message );
	}
	return;
}

void KIMProxy::sendFile(const QString &uid, const KURL &sourceURL, const QString &altFileName, uint fileSize )
{
	if ( initialize() )
	{
		QDictIterator<KIMIface_stub> it( m_im_client_stubs );
		for ( ; it.current(); ++it )
		{
			if ( it.current()->canReceiveFiles( uid ) )
				it.current()->sendFile( uid, sourceURL, altFileName, fileSize );
		}	
	}
	return;
}

bool KIMProxy::addContact( const QString &contactId, const QString &protocol )
{
	if ( initialize() )
	{
		return stubForProtocol( protocol )->addContact( contactId, protocol );
	}
	return false;
}

QString KIMProxy::locate( const QString & contactId, const QString & protocol )
{
	if ( initialize() )
	{
		return stubForProtocol( protocol )->locate( contactId, protocol );
	}
	return QString::null;
}

bool KIMProxy::imAppsAvailable()
{
	kdDebug( 5301 ) << k_funcinfo << " returning " << m_apps_available<< endl;
	return m_apps_available;
}

bool KIMProxy::startPreferredApp()
{
	QString preferences = QString("[X-DCOP-ServiceName] = '%1'").arg( preferredApp() );
	// start/find an instance of DCOP/InstantMessenger
	QString error;
	QCString dcopService;
	// Get a preferred IM client.
	// The app will notify itself to us using registeredToDCOP, so we don't need to record a stub for it here
	// FIXME: error in preferences, see debug output
	preferences = QString::null;
	int result = KDCOPServiceStarter::self()->findServiceFor( IM_SERVICE_TYPE, QString::null, preferences, &error, &dcopService );

	kdDebug( 5301 ) << k_funcinfo << "error was: " << error << ", dcopService: " << dcopService << endl;

	return ( result == 0 );
}

void KIMProxy::unregisteredFromDCOP( const QCString& appId )
{
	kdDebug( 5301 ) << k_funcinfo << appId << endl;
	if ( m_im_client_stubs.find( appId ) )
	{
		// invalidate all
		QDictIterator<AppPresence> it( m_presence_map ); 
		for ( ; it.current(); ++it )
		{
		if ( it.current()->appId == QString( appId ) )
		{  m_presence_map.remove( it.currentKey() );
			break;
		}
		}
		m_im_client_stubs.remove( appId );
		emit sigPresenceInfoExpired();
	}
}

void KIMProxy::registeredToDCOP( const QCString& appId )
{
	// check that appId implements our service
	// if the appId ends with a number, i.e. a pid like in foobar-12345,
	// there's no chance
	if ( appId.isEmpty() || QChar( appId[ appId.length() - 1 ] ).isDigit() )
		return;

	kdDebug( 5301 ) << k_funcinfo << appId << endl;
	bool newApp = false;
	// get an up to date list of offers in case a new app was installed
	const KService::List offers = KServiceType::offers( IM_SERVICE_TYPE );
	KService::List::const_iterator it;
	for ( it = offers.begin(); it != offers.end(); ++it )
	{
		QCString dcopObjectId = "KIMIface";
		QCString dcopService = (*it)->property("X-DCOP-ServiceName").toString().latin1();
		kdDebug( 5301 ) << "dcopService: " << dcopService << ", appId: " << appId << endl;
		if ( dcopService == appId )
		{
			m_apps_available = true;
			// if it's not already known, insert it
			if ( !m_im_client_stubs.find( appId ) )
			{	
				newApp = true;
				m_im_client_stubs.insert( dcopService, new KIMIface_stub( m_dc, appId, dcopObjectId ) );
			}
		}
		else
			kdDebug( 5301 ) << "App doesn't implement our ServiceType" << endl;
	}
	if ( newApp )
		emit sigPresenceInfoExpired();
}

void KIMProxy::contactPresenceChanged( QString uid, QCString appId, int presence )
{
	// update the presence map
	kdDebug( 5301 ) << k_funcinfo << "uid: " << uid << " appId: " << appId << " presence " << presence << endl;
	if ( updatePresence( uid, appId, presence ) )
		emit sigContactPresenceChanged( uid );
}

bool KIMProxy::updatePresence( const QString &uid, const QString &appId, int presence )
{
	// if exists
	AppPresence *existing = 0L;
	if ( ( existing = m_presence_map.find( uid ) ) )
	{
		// update to best, where best = same app as current presence, or higher presence from different app
		if ( presence > existing->presence || appId == existing->appId )
		{
			existing->appId = appId;
			existing->presence = presence;
			return true;
		}
	}
	else
	{
		// else insert
		AppPresence *newPresence = new AppPresence;
		newPresence->appId = appId;
		newPresence->presence = presence;
		m_presence_map.insert( uid, newPresence );
	}
	return false;
}

void KIMProxy::pollAll( const QString &uid )
{
	// We only need to call this function if we don't have any data at all
	// otherwise, the data will be kept fresh by received presence change
	// DCOP signals
	if ( !m_presence_map.find( uid ) )
	{
		// find best presence from known clients
		QString bestAppId;
		int bestPresence;
		QDictIterator<KIMIface_stub> it( m_im_client_stubs );
		
		// start with the first entry as baseline
		bestAppId = it.currentKey();
		bestPresence = it.current()->presenceStatus( uid );
		
		// now check the others and see if they have a better presence
		++it;
		for ( ; it.current(); ++it )
		{
			int currentPresence = it.current()->presenceStatus( uid );
			if ( currentPresence > bestPresence )
			{
				bestPresence = currentPresence;
				bestAppId = it.currentKey();
			}
		}
		
		// record the best presence
		updatePresence( uid, bestAppId, bestPresence );
	}
}

KIMIface_stub * KIMProxy::stubForUid( const QString &uid )
{
	pollAll( uid );
	// get best appPresence
	
	AppPresence *ap = m_presence_map.find( uid );
	// look up the presence string from that app
	if ( ap )
		return m_im_client_stubs.find( ap->appId );
	else
		return 0L;
}

KIMIface_stub * KIMProxy::stubForProtocol( const QString &protocol)
{
	KIMIface_stub * app;
	// see if the preferred client supports this protocol
	QString preferred = preferredApp();
	if ( ( app = m_im_client_stubs.find( preferred ) ) )
	{
		if ( app->protocols().grep( protocol ).count() > 0 )
			return app;
	}
	// preferred doesn't do this protocol, try the first of the others that says it does
	QDictIterator<KIMIface_stub> it( m_im_client_stubs );
	for ( ; it.current(); ++it )
	{
		if ( it.current()->protocols().grep( protocol ).count() > 0 )
			return it.current();
	}	
	return 0L;
}

QString KIMProxy::preferredApp()
{
	KConfig *store = new KSimpleConfig( IM_CLIENT_PREFERENCES_FILE );
	store->setGroup( IM_CLIENT_PREFERENCES_SECTION );
	QString preferredApp = store->readEntry( IM_CLIENT_PREFERENCES_ENTRY );
	kdDebug( 5301 ) << k_funcinfo << "found preferred app: " << preferredApp << endl;
	return preferredApp;
}	

#include "kimproxy.moc"

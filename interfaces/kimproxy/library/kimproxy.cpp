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

#include <qglobal.h>
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

int bestPresence( AppPresence* ap )
{
	Q_ASSERT( ap );
	AppPresence::const_iterator it;
	it = ap->begin();
	int best = 0; // unknown
	if ( it != ap->end() )
	{
		best = it.data();
		++it;
		for ( ; it != ap->end(); ++it )
		{
			if ( it.data() > best )
				best = it.data(); 
		}
	}
	return best;
}

QCString bestAppId( AppPresence* ap )
{
	Q_ASSERT( ap );
	AppPresence::const_iterator it;
	QCString bestAppId;
	it = ap->begin();
	if ( it != ap->end() )
	{
		int best = it.data();
		bestAppId = it.key();
		++it;
		for ( ; it != ap->end(); ++it )
		{
			if ( it.data() > best )
			{
				best = it.data();
				bestAppId = it.key();
			}
		}
	}
	return bestAppId;
}

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
	m_presence_map.setAutoDelete( true );

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
	//m_dc->setNotifications( false );
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
			KService::List::iterator offer;
			typedef QValueList<QCString> QCStringList;
			QCStringList registeredApps = m_dc->registeredApplications();
			QCStringList::iterator app;
			const QCStringList::iterator end = registeredApps.end();
			// for each registered app
			for ( app = registeredApps.begin(); app != end; ++app )
			{
				//kdDebug( 5301 ) << " considering: " << *app << endl;
				//for each offer
				for ( offer = offers.begin(); offer != offers.end(); ++offer )
				{
					QCString dcopService = (*offer)->property("X-DCOP-ServiceName").toString().latin1();
					if ( !dcopService.isEmpty() )
					{
						//kdDebug( 5301 ) << " is it: " << dcopService << "?" << endl;
						// get the application name ( minus any process ID )
						QCString instanceName =  (*app).left( dcopService.length() );
						// if the application implements the dcop service, add it 
						if ( instanceName == dcopService )
						{
							m_apps_available = true;
							//kdDebug( 5301 ) << " app name: " << (*offer)->name() << ", has instance " << *app << ", dcopService: " << dcopService << endl;
							if ( !m_im_client_stubs.find( dcopService ) )
							{
								kdDebug( 5301 ) << "inserting new stub for " << *app << " dcopObjectId " << dcopObjectId << endl;
								m_im_client_stubs.insert( *app, new KIMIface_stub( m_dc, *app, dcopObjectId ) );
							}
						}
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
		if ( KIMIface_stub* s = stubForUid( uid ) )
			name = s->displayName( uid );
	}
	//kdDebug( 5301 ) << k_funcinfo << name << endl;
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
			presence = bestPresence( ap );
	}
	return presence;
}

QString KIMProxy::presenceString( const QString& uid )
{
	//kdDebug( 5301 ) << k_funcinfo << endl;
	
	QString presence;
	KIMIface_stub* s = stubForUid( uid );
	if ( initialize() && s )
	{
		
		PresenceStringMap * appPresenceStrings = m_client_presence_strings[ s ];
		if ( !appPresenceStrings ) // we have no presence strings mapped at all for this stub
		{
			//kdDebug( 5301 ) << " no string cache found for this stub , creating one" << endl;
			appPresenceStrings = new PresenceStringMap();
			m_client_presence_strings.insert( s, appPresenceStrings );
		}
		int numeric = presenceNumeric( uid );
		presence = (*appPresenceStrings)[ numeric ];
		if ( presence.isEmpty() ) //  cache miss
		{
			//kdDebug( 5301 ) << " no cached string found for this app, fetching it" << endl;
			presence = s->presenceString( uid );
			appPresenceStrings->insert( numeric, presence );
		}
		//kdDebug( 5301 ) << " resulting presence string for " << uid << " : " << presence << endl;
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
		QCString ba = bestAppId( existing );
		if ( !ba.isNull() )
		{
			QString appPresenceKey = QString::number( presenceNumeric( uid ) ).append( ba );
			if ( !QPixmapCache::find( appPresenceKey, presence ) )
			{
				if ( KIMIface_stub* s = stubForUid( uid ) )
				{
					presence = s->icon( uid );
					QPixmapCache::insert( appPresenceKey, presence );
				}
			}
		}
	}
	return presence;
}

bool KIMProxy::canReceiveFiles( const QString & uid )
{
	if ( initialize() )
	{
		if ( KIMIface_stub* s = stubForUid( uid ) )
			return s->canReceiveFiles( uid );
	}
	return false;
}

bool KIMProxy::canRespond( const QString & uid )
{
	if ( initialize() )
	{
		if ( KIMIface_stub* s = stubForUid( uid ) )
			return s->canRespond( uid );
	}
	return false;
}

QString KIMProxy::context( const QString & uid )
{
	if ( initialize() )
	{
		if ( KIMIface_stub* s = stubForUid( uid ) )
			return s->context( uid );
	}
	return QString::null;
}
	
void KIMProxy::chatWithContact( const QString& uid )
{
	if ( initialize() )
	{
		if ( KIMIface_stub* s = stubForUid( uid ) )
			s->chatWithContact( uid );
	}
	return;
}

void KIMProxy::messageContact( const QString& uid, const QString& message )
{
	if ( initialize() )
	{
		if ( KIMIface_stub* s = stubForUid( uid ) )
			s->messageContact( uid, message );
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
			{
				it.current()->sendFile( uid, sourceURL, altFileName, fileSize );
				break;
			}
		}	
	}
	return;
}

bool KIMProxy::addContact( const QString &contactId, const QString &protocol )
{
	if ( initialize() )
	{
		if ( KIMIface_stub* s = stubForProtocol( protocol ) )
		return s->addContact( contactId, protocol );
	}
	return false;
}

QString KIMProxy::locate( const QString & contactId, const QString & protocol )
{
	if ( initialize() )
	{
		if ( KIMIface_stub* s = stubForProtocol( protocol ) )
		return s->locate( contactId, protocol );
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
		kdDebug( 5301 ) << "removing references to " << appId << endl;
		// invalidate all
		QDictIterator<AppPresence> it( m_presence_map ); 
		for ( ; it.current(); ++it )
		{
			AppPresence::iterator apIt;
			for ( apIt = it.current()->begin(); apIt != it.current()->end(); ++apIt )
				if ( apIt.key() == appId )
				{
					it.current()->remove( apIt );
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
	// there's no chance - why did i make this decision
	if ( appId.isEmpty() ) /*|| QChar( appId[ appId.length() - 1 ] ).isDigit() ) */
		return;
	// So there is no error from a failed query when using kdelibs 3.2, which don't have this servicetype
	if ( !KServiceType::serviceType( IM_SERVICE_TYPE ) ) 
		return;

	kdDebug( 5301 ) << k_funcinfo << appId << endl;
	bool newApp = false;
	// get an up to date list of offers in case a new app was installed
	const KService::List offers = KServiceType::offers( IM_SERVICE_TYPE );
	KService::List::const_iterator it;
	// check each of the offers that implement the service type we're lookin for,
	// to see if any of them are the app that just registered
	for ( it = offers.begin(); it != offers.end(); ++it )
	{
		QCString dcopObjectId = "KIMIface";
		QCString dcopService = (*it)->property("X-DCOP-ServiceName").toString().latin1();
		kdDebug( 5301 ) << "dcopService: " << dcopService << ", appId: " << appId << endl;
		if ( appId.left( dcopService.length() ) == dcopService )
		{
			m_apps_available = true;
			// if it's not already known, insert it
			if ( !m_im_client_stubs.find( appId ) )
			{	
				newApp = true;
				m_im_client_stubs.insert( appId, new KIMIface_stub( m_dc, appId, dcopObjectId ) );
			}
		}
		//else
		//	kdDebug( 5301 ) << "App doesn't implement our ServiceType" << endl;
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

bool KIMProxy::updatePresence( const QString &uid, const QCString &appId, int presence )
{
	// if exists	
	AppPresence *userPresences = 0L;
	if ( ( userPresences = m_presence_map.find( uid ) ) )
	{
		// get the old presence
		int oldBest = bestPresence( userPresences );
		QCString oldBestApp = bestAppId( userPresences );
		// update the presence
		userPresences->insert( appId, presence );
		int newBest =  bestPresence( userPresences );
		// return if the update is better than the old presence
		return ( newBest > oldBest || appId == oldBestApp );
	}
	else
	{
		// else insert
		userPresences = new AppPresence();
		userPresences->insert( appId, presence );
		m_presence_map.insert( uid, userPresences );
		return true;
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
		AppPresence *presence = new AppPresence();
		// record current presence from known clients
		QDictIterator<KIMIface_stub> it( m_im_client_stubs );
		for ( ; it.current(); ++it )
		{
			presence->insert( it.currentKey().ascii(), it.current()->presenceStatus( uid ) ); // m_im_client_stubs has qstring keys...
		}
		m_presence_map.insert( uid, presence );
	}
}

KIMIface_stub * KIMProxy::stubForUid( const QString &uid )
{
	pollAll( uid );
	// get best appPresence
	
	AppPresence *ap = m_presence_map.find( uid );
	// look up the presence string from that app
	if ( ap )
		return m_im_client_stubs.find( bestAppId( ap ) );
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
	//kdDebug( 5301 ) << k_funcinfo << "found preferred app: " << preferredApp << endl;
	return preferredApp;
}	

#include "kimproxy.moc"

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <qglobal.h>
#include <qpixmapcache.h>


#include <dcopclient.h>
#include <kapplication.h>
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

struct AppPresenceCurrent
{
	DCOPCString appId;
	int presence;
};

class ContactPresenceListCurrent : public QList<AppPresenceCurrent>
{
	public:
		// return value indicates if the supplied parameter was better than any existing presence
		bool update( const AppPresenceCurrent );
		AppPresenceCurrent best();
};


struct KIMProxy::Private
{
	DCOPClient * dc;
	// list of the strings in use by KIMIface
	QStringList presence_strings;
	// list of the icon names in use by KIMIface
	QStringList presence_icons;
	// map of presences
	PresenceStringMap presence_map;
};

bool ContactPresenceListCurrent::update( AppPresenceCurrent ap )
{
	if ( isEmpty() )
	{
		append( ap );
		return true;
	}
	
	bool bestChanged = false;
	AppPresenceCurrent best;
	best.presence = -1;
	ContactPresenceListCurrent::iterator it = begin();
	const ContactPresenceListCurrent::iterator itEnd = end();
	ContactPresenceListCurrent::iterator existing = itEnd;

	while ( it != itEnd )
	{
		if ( (*it).presence > best.presence )
			best = (*it);
		if ( (*it).appId == ap.appId )
			existing = it;
		++it;
	}
	
	if ( ap.presence > best.presence ||
      best.appId == ap.appId )
		bestChanged = true;
	
	if ( existing != itEnd )
	{
		remove( existing );
		append( ap );
	}
	return bestChanged;
}

AppPresenceCurrent ContactPresenceListCurrent::best()
{
	AppPresenceCurrent best;
	best.presence = -1;
	ContactPresenceListCurrent::iterator it = begin();
	const ContactPresenceListCurrent::iterator itEnd = end();
	while ( it != itEnd )
	{
		if ( (*it).presence > best.presence )
			best = (*it);
		++it;
	}
	// if it's still -1 here, we have no presence data, so we return Unknown
	if ( best.presence == -1 )
		best.presence = 0;
	return best;
}

// int bestPresence( AppPresence* ap )
// {
// 	Q_ASSERT( ap );
// 	AppPresence::const_iterator it;
// 	it = ap->begin();
// 	int best = 0; // unknown
// 	if ( it != ap->end() )
// 	{
// 		best = it.data();
// 		++it;
// 		for ( ; it != ap->end(); ++it )
// 		{
// 			if ( it.data() > best )
// 				best = it.data(); 
// 		}
// 	}
// 	return best;
// }
// 
// QCString bestAppId( AppPresence* ap )
// {
// 	Q_ASSERT( ap );
// 	AppPresence::const_iterator it;
// 	QCString bestAppId;
// 	it = ap->begin();
// 	if ( it != ap->end() )
// 	{
// 		int best = it.data();
// 		bestAppId = it.key();
// 		++it;
// 		for ( ; it != ap->end(); ++it )
// 		{
// 			if ( it.data() > best )
// 			{
// 				best = it.data();
// 				bestAppId = it.key();
// 			}
// 		}
// 	}
// 	return bestAppId;
// }

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

KIMProxy::KIMProxy( DCOPClient* dc ) : DCOPObject( "KIMProxyIface" ), QObject(), d( new Private )
{
	m_im_client_stubs.setAutoDelete( true );

	d->dc = dc;
	m_initialized = false;
	connect( d->dc, SIGNAL( applicationRemoved( const QByteArray& ) ) , this, SLOT( unregisteredFromDCOP( const QByteArray& ) ) );
	connect( d->dc, SIGNAL( applicationRegistered( const QByteArray& ) ) , this, SLOT( registeredToDCOP( const QByteArray& ) ) );
	d->dc->setNotifications( true );

	d->presence_strings.append( "Unknown" );
	d->presence_strings.append( "Offline" );
	d->presence_strings.append( "Connecting" );
	d->presence_strings.append( "Away" );
	d->presence_strings.append( "Online" );
	
	d->presence_icons.append( "presence_unknown" );
	d->presence_icons.append( "presence_offline" );
	d->presence_icons.append( "presence_connecting" );
	d->presence_icons.append( "presence_away" );
	d->presence_icons.append( "presence_online" );
	
	//QCString senderApp = "Kopete";
	//QCString senderObjectId = "KIMIface";
	DCOPCString method = "contactPresenceChanged( QString, QCString, int )";
	//QCString receiverObjectId = "KIMProxyIface";
	
	// FIXME: make this work when the sender object id is set to KIMIFace
	if ( !connectDCOPSignal( 0, 0, method, method, false ) )
		KMessageBox::information( 0, QString( "Couldn't connect DCOP signal.\nWon't receive any status notifications!" ) );
}

KIMProxy::~KIMProxy( )
{
	//d->dc->setNotifications( false );
}

bool KIMProxy::initialize()
{
	if ( !m_initialized )
	{
		m_initialized = true; // we should only do this once, as registeredToDCOP() will catch any new starts
		// So there is no error from a failed query when using kdelibs 3.2, which don't have this servicetype
		if ( KServiceType::serviceType( IM_SERVICE_TYPE ) ) 
		{
			//kDebug( 790 ) << k_funcinfo << endl;
			DCOPCString dcopObjectId = "KIMIface";
	
			// see what apps implementing our service type are out there
			KService::List offers = KServiceType::offers( IM_SERVICE_TYPE );
			KService::List::iterator offer;
			DCOPCStringList registeredApps = d->dc->registeredApplications();
			DCOPCStringList::iterator app;
			const DCOPCStringList::iterator end = registeredApps.end();
			// for each registered app
			for ( app = registeredApps.begin(); app != end; ++app )
			{
				//kDebug( 790 ) << " considering: " << *app << endl;
				//for each offer
				for ( offer = offers.begin(); offer != offers.end(); ++offer )
				{
					DCOPCString dcopService = (*offer)->property("X-DCOP-ServiceName").toString().latin1();
					if ( !dcopService.isEmpty() )
					{
						//kDebug( 790 ) << " is it: " << dcopService << "?" << endl;
						// get the application name ( minus any process ID )
						DCOPCString instanceName =  (*app).left( dcopService.length() );
						// if the application implements the dcop service, add it 
						if ( instanceName == dcopService )
						{
							m_apps_available = true;
							//kDebug( 790 ) << " app name: " << (*offer)->name() << ", has instance " << *app << ", dcopService: " << dcopService << endl;
							if ( !m_im_client_stubs.find( dcopService ) )
							{
								kDebug( 790 ) << "App " << *app << ", dcopObjectId " << dcopObjectId << " found, using it for presence info." << endl;
								m_im_client_stubs.insert( *app, new KIMIface_stub( d->dc, *app, dcopObjectId ) );
								pollApp( *app );
							}
						}
					}
				}
			}
		}
	}
	return !m_im_client_stubs.isEmpty();
}

void KIMProxy::registeredToDCOP( const QByteArray& appId )
{
	//kDebug( 790 ) << k_funcinfo << " appId '" << appId << "'" << endl;
	// check that appId implements our service
	// if the appId ends with a number, i.e. a pid like in foobar-12345,
	if ( appId.isEmpty() )
		return;

	bool newApp = false;
	// get an up to date list of offers in case a new app was installed
	// and check each of the offers that implement the service type we're looking for,
	// to see if any of them are the app that just registered
	const KService::List offers = KServiceType::offers( IM_SERVICE_TYPE );
	KService::List::const_iterator it;
	for ( it = offers.begin(); it != offers.end(); ++it )
	{
		DCOPCString dcopObjectId = "KIMIface";
		DCOPCString dcopService = (*it)->property("X-DCOP-ServiceName").toString().latin1();
		if ( appId.left( dcopService.length() ) == dcopService )
		{
			// if it's not already known, insert it
			if ( !m_im_client_stubs.find( appId ) )
			{
				newApp = true;
				kDebug( 790 ) << "App: " << appId << ", dcopService: " << dcopService << " started, using it for presence info."<< endl;
				m_im_client_stubs.insert( appId, new KIMIface_stub( d->dc, appId, dcopObjectId ) );
			}
		}
		//else
		//	kDebug( 790 ) << "App doesn't implement our ServiceType" << endl;
	}
	//if ( newApp )
	//	emit sigPresenceInfoExpired();
}

void KIMProxy::unregisteredFromDCOP( const QByteArray& appId )
{
	//kDebug( 790 ) << k_funcinfo << appId << endl;
	if ( m_im_client_stubs.find( appId ) )
	{
		kDebug( 790 ) << appId << " quit, removing its presence info." << endl;
		
		PresenceStringMap::Iterator it = d->presence_map.begin();
		const PresenceStringMap::Iterator end = d->presence_map.end();
		for ( ; it != end; ++it )
		{
			ContactPresenceListCurrent list = it.data();
			ContactPresenceListCurrent::iterator cpIt = list.begin();
			while( cpIt != list.end() )
			{
				ContactPresenceListCurrent::iterator gone = cpIt++;
				if ( (*gone).appId == appId )
				{
					list.remove( gone );
				}
			}
		}
		m_im_client_stubs.remove( appId );
		emit sigPresenceInfoExpired();
	}
}

void KIMProxy::contactPresenceChanged( QString uid, DCOPCString appId, int presence )
{
	// update the presence map
	//kDebug( 790 ) << k_funcinfo << "uid: " << uid << " appId: " << appId << " presence " << presence << endl;
	ContactPresenceListCurrent current;
	current = d->presence_map[ uid ];
  //kDebug( 790 ) << "current best presence from : " << current.best().appId << " is: " << current.best().presence << endl;
	AppPresenceCurrent newPresence;
	newPresence.appId = appId;
	newPresence.presence = presence;

	if ( current.update( newPresence ) )
	{
		d->presence_map.insert( uid, current );
		emit sigContactPresenceChanged( uid );
	}
}

int KIMProxy::presenceNumeric( const QString& uid )
{
	AppPresenceCurrent ap;
	ap.presence = 0;
	if ( initialize() )
	{
		ContactPresenceListCurrent presence = d->presence_map[ uid ];
		ap = presence.best();
	}
	return ap.presence;
}

QString KIMProxy::presenceString( const QString& uid )
{
	AppPresenceCurrent ap;
	ap.presence = 0;
	if ( initialize() )
	{
		ContactPresenceListCurrent presence = d->presence_map[ uid ];
		ap = presence.best();
	}
	if ( ap.appId.isEmpty() )
		return QString();
	else
		return d->presence_strings[ ap.presence ];
}

QPixmap KIMProxy::presenceIcon( const QString& uid )
{
	AppPresenceCurrent ap;
	ap.presence = 0;
	if ( initialize() )
	{
		ContactPresenceListCurrent presence = d->presence_map[ uid ];
		ap = presence.best();
	}
	if ( ap.appId.isEmpty() )
	{
		//kDebug( 790 ) << k_funcinfo << "returning a null QPixmap because we were asked for an icon for a uid we know nothing about" << endl;
		return QPixmap();
	}
	else
	{
		//kDebug( 790 ) << k_funcinfo << "returning this: " << d->presence_icons[ ap.presence ] << endl;
		return SmallIcon( d->presence_icons[ ap.presence ]);
	}
}

QStringList KIMProxy::allContacts()
{
	QStringList value = d->presence_map.keys();
	return value;
}

QStringList KIMProxy::reachableContacts()
{
	QStringList value;
	
	if ( initialize() )
	{
		Q3DictIterator<KIMIface_stub> it( m_im_client_stubs );
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
	PresenceStringMap::iterator it = d->presence_map.begin();
	const PresenceStringMap::iterator end= d->presence_map.end();
	for ( ; it != end; ++it )
		if ( it.data().best().presence > 2 /*Better than Connecting, ie Away or Online*/ )
			value.append( it.key() );
		
	return value;
}

QStringList KIMProxy::fileTransferContacts()
{
	QStringList value;
	
	if ( initialize() )
	{
		Q3DictIterator<KIMIface_stub> it( m_im_client_stubs );
		for ( ; it.current(); ++it )
		{
			value += it.current()->fileTransferContacts( );
		}
	}
	return value;
}

bool KIMProxy::isPresent( const QString& uid )
{
	return ( !d->presence_map[ uid ].isEmpty() );
}

QString KIMProxy::displayName( const QString& uid )
{
	QString name;
	if ( initialize() )
	{
		if ( KIMIface_stub* s = stubForUid( uid ) )
			name = s->displayName( uid );
	}
	//kDebug( 790 ) << k_funcinfo << name << endl;
    return name;
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
	return QString();
}
	
void KIMProxy::chatWithContact( const QString& uid )
{
	if ( initialize() )
	{
		if ( KIMIface_stub* s = stubForUid( uid ) )
		{
			kapp->updateRemoteUserTimestamp( s->app() );
			s->chatWithContact( uid );
		}
	}
	return;
}

void KIMProxy::messageContact( const QString& uid, const QString& message )
{
	if ( initialize() )
	{
		if ( KIMIface_stub* s = stubForUid( uid ) )
		{
			kapp->updateRemoteUserTimestamp( s->app() );
			s->messageContact( uid, message );
		}
	}
	return;
}

void KIMProxy::sendFile(const QString &uid, const KUrl &sourceURL, const QString &altFileName, uint fileSize )
{
	if ( initialize() )
	{
		Q3DictIterator<KIMIface_stub> it( m_im_client_stubs );
		for ( ; it.current(); ++it )
		{
			if ( it.current()->canReceiveFiles( uid ) )
			{
				kapp->updateRemoteUserTimestamp( it.current()->app() );
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
	return QString();
}

bool KIMProxy::imAppsAvailable()
{
	return ( !m_im_client_stubs.isEmpty() );
}

bool KIMProxy::startPreferredApp()
{
	QString preferences = QString("[X-DCOP-ServiceName] = '%1'").arg( preferredApp() );
	// start/find an instance of DCOP/InstantMessenger
	QString error;
	DCOPCString dcopService;
	// Get a preferred IM client.
	// The app will notify itself to us using registeredToDCOP, so we don't need to record a stub for it here
	// FIXME: error in preferences, see debug output
	preferences.clear();
	int result = KDCOPServiceStarter::self()->findServiceFor( IM_SERVICE_TYPE, QString(), preferences, &error, &dcopService );

	kDebug( 790 ) << k_funcinfo << "error was: " << error << ", dcopService: " << dcopService << endl;

	return ( result == 0 );
}


void KIMProxy::pollAll( const QString &uid )
{
/*	// We only need to call this function if we don't have any data at all
	// otherwise, the data will be kept fresh by received presence change
	// DCOP signals
	if ( !d->presence_map.contains( uid ) )
	{
		AppPresence *presence = new AppPresence();
		// record current presence from known clients
		QDictIterator<KIMIface_stub> it( m_im_client_stubs );
		for ( ; it.current(); ++it )
		{
			presence->insert( it.currentKey().ascii(), it.current()->presenceStatus( uid ) ); // m_im_client_stubs has qstring keys...
		}
		d->presence_map.insert( uid, presence );
	}*/
}

void KIMProxy::pollApp( const DCOPCString & appId )
{
	//kDebug( 790 ) << k_funcinfo << endl;
	KIMIface_stub * appStub = m_im_client_stubs[ appId ];
	QStringList contacts = m_im_client_stubs[ appId ]->allContacts();
	QStringList::iterator it = contacts.begin();
	QStringList::iterator end = contacts.end();
	for ( ; it != end; ++it )
	{
		ContactPresenceListCurrent current = d->presence_map[ *it ];
		AppPresenceCurrent ap;
		ap.appId = appId;
		ap.presence = appStub->presenceStatus( *it );
		current.append( ap );

		d->presence_map.insert( *it, current );
		if ( current.update( ap ) )
			emit sigContactPresenceChanged( *it );
		//kDebug( 790 ) << " uid: " << *it << " presence: " << ap.presence << endl;
	}
}

KIMIface_stub * KIMProxy::stubForUid( const QString &uid )
{
	// get best appPresence
	AppPresenceCurrent ap = d->presence_map[ uid ].best();
	// look up the presence string from that app
		return m_im_client_stubs.find( ap.appId );
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
	Q3DictIterator<KIMIface_stub> it( m_im_client_stubs );
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
	//kDebug( 790 ) << k_funcinfo << "found preferred app: " << preferredApp << endl;
	return preferredApp;
}	

#include "kimproxy.moc"

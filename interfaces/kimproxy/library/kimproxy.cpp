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

#include "kimproxy.h"

#include <QtCore/QBool>
#include <QtGui/QPixmapCache>

#include <kapplication.h>
#include <kdbusservicestarter.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kiconloader.h>
#include <kservice.h>
#include <kservicetypetrader.h>

#include "kimiface.h"

#include <kservicetype.h>

struct AppPresenceCurrent
{
	QString appId;
	int presence;
};

static int debugArea() {
    static int s_area = KDebug::registerArea("kimproxy (kdelibs)");
    return s_area;
}

class ContactPresenceListCurrent : public QList<AppPresenceCurrent>
{
	public:
		// return value indicates if the supplied parameter was better than any existing presence
		bool update( const AppPresenceCurrent );
		AppPresenceCurrent best();
};


class KIMProxy::Private
{
public:
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
		erase( existing );
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

OrgKdeKIMInterface * findInterface( const QString & app )
{
        return new OrgKdeKIMInterface( app, "/KIMIface", QDBusConnection::sessionBus() );
}

KIMProxy * KIMProxy::instance()
{
	K_GLOBAL_STATIC(KIMProxy, s_instance)
	return s_instance;
}

KIMProxy::KIMProxy() : QObject(), d( new Private )
{
	//QDBus::sessionBus().registerObject( "/KIMProxy", this);
	m_initialized = false;
	connect( QDBusConnection::sessionBus().interface(),
		 SIGNAL(serviceOwnerChanged(QString,QString,QString)),
		 SLOT(nameOwnerChanged(QString,QString,QString)) );

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
	//DCOPCString method = "contactPresenceChanged( QString, QCString, int )";
	//QCString receiverObjectId = "KIMProxyIface";

	QDBusConnection::sessionBus().connect( QString(), "/KIMIface", "org.kde.KIM", "contactPresenceChanged",
				     this, SLOT(contactPresenceChanged(QString,QString,int)) );
}

KIMProxy::~KIMProxy( )
{
	qDeleteAll(m_im_client_stubs);
}

bool KIMProxy::initialize()
{
	if ( !m_initialized )
	{
		m_initialized = true; // we should only do this once, as registeredToDCOP() will catch any new starts
		// So there is no error from a failed query when using kdelibs 3.2, which don't have this servicetype
		if ( KServiceType::serviceType( IM_SERVICE_TYPE ) )
		{
			// see what apps implementing our service type are out there
			const KService::List offers = KServiceTypeTrader::self()->query( IM_SERVICE_TYPE );
			KService::List::const_iterator offer;
			QStringList registeredApps = QDBusConnection::sessionBus().interface()->registeredServiceNames();
			foreach (const QString &app, registeredApps)
			{
				//kDebug( debugArea() ) << " considering: " << *app;
				//for each offer
				for ( offer = offers.begin(); offer != offers.end(); ++offer )
				{
					QString dbusService = (*offer)->property("X-DBUS-ServiceName").toString();
					if ( !dbusService.isEmpty() )
					{
						//kDebug( debugArea() ) << " is it: " << dbusService << "?";
						// if the application implements the dcop service, add it
						if ( app.startsWith( dbusService ) )
						{
							m_apps_available = true;
							//kDebug( debugArea() ) << " app name: " << (*offer)->name() << ", has instance " << *app << ", dbusService: " << dbusService;
							if ( !m_im_client_stubs.contains( dbusService ) )
							{
								kDebug( debugArea() ) << "App " << app << ", found, using it for presence info.";
								m_im_client_stubs.insert( app, findInterface( app ) );
								pollApp( app );
							}
						}
					}
				}
			}
		}
	}
	return !m_im_client_stubs.isEmpty();
}

void KIMProxy::nameOwnerChanged( const QString & appId, const QString &, const QString & newOwner )
{
	// unregister...
	if ( m_im_client_stubs.contains( appId ) )
	{
		kDebug( debugArea() ) << appId << " quit, removing its presence info.";

		PresenceStringMap::Iterator it = d->presence_map.begin();
		const PresenceStringMap::Iterator end = d->presence_map.end();
		for ( ; it != end; ++it )
		{
			ContactPresenceListCurrent list = it.value();
			ContactPresenceListCurrent::iterator cpIt = list.begin();
			while( cpIt != list.end() )
			{
				ContactPresenceListCurrent::iterator gone = cpIt++;
				if ( (*gone).appId == appId )
				{
					list.erase( gone );
				}
			}
		}
		delete m_im_client_stubs.take( appId );
		emit sigPresenceInfoExpired();
	}

	// reregister...
	if ( !newOwner.isEmpty() ) { // application registered
		bool newApp = false;
		// get an up to date list of offers in case a new app was installed
		// and check each of the offers that implement the service type we're looking for,
		// to see if any of them are the app that just registered
		const KService::List offers = KServiceTypeTrader::self()->query( IM_SERVICE_TYPE );
		KService::List::const_iterator it;
		for ( it = offers.begin(); it != offers.end(); ++it )
		{
			QString dbusService = (*it)->property("X-DBUS-ServiceName").toString();
			if ( appId.startsWith( dbusService ) )
			{
				// if it's not already known, insert it
				if ( !m_im_client_stubs.contains( appId ) )
				{
					newApp = true;
					kDebug( debugArea() ) << "App: " << appId << ", dbusService: " << dbusService << " started, using it for presence info.";
					m_im_client_stubs.insert( appId, findInterface( appId ) );
				}
			}
			//else
			//	kDebug( debugArea() ) << "App doesn't implement our ServiceType";
		}
		//if ( newApp )
		//	emit sigPresenceInfoExpired();
	}
}

void KIMProxy::contactPresenceChanged( const QString& uid, const QString& appId, int presence )
{
	// update the presence map
	//kDebug( debugArea() ) << "uid: " << uid << " appId: " << appId << " presence " << presence;
	ContactPresenceListCurrent current;
	current = d->presence_map[ uid ];
  //kDebug( debugArea() ) << "current best presence from : " << current.best().appId << " is: " << current.best().presence;
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
		//kDebug( debugArea() ) << "returning a null QPixmap because we were asked for an icon for a uid we know nothing about";
		return QPixmap();
	}
	else
	{
		//kDebug( debugArea() ) << "returning this: " << d->presence_icons[ ap.presence ];
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
		QHashIterator<QString, OrgKdeKIMInterface*> it( m_im_client_stubs );
		while (it.hasNext())
		{
			it.next();
			value += it.value()->reachableContacts( );
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
		if ( it.value().best().presence > 2 /*Better than Connecting, ie Away or Online*/ )
			value.append( it.key() );

	return value;
}

QStringList KIMProxy::fileTransferContacts()
{
	QStringList value;

	if ( initialize() )
	{
		QHashIterator<QString, OrgKdeKIMInterface*> it( m_im_client_stubs );
		while (it.hasNext())
		{
			it.next();
			value += it.value()->fileTransferContacts( );
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
		if ( OrgKdeKIMInterface* s = stubForUid( uid ) )
			name = s->displayName( uid );
	}
	//kDebug( debugArea() ) << name;
    return name;
}

bool KIMProxy::canReceiveFiles( const QString & uid )
{
	if ( initialize() )
	{
		if ( OrgKdeKIMInterface* s = stubForUid( uid ) )
			return s->canReceiveFiles( uid );
	}
	return false;
}

bool KIMProxy::canRespond( const QString & uid )
{
	if ( initialize() )
	{
		if ( OrgKdeKIMInterface* s = stubForUid( uid ) )
			return s->canRespond( uid );
	}
	return false;
}

QString KIMProxy::context( const QString & uid )
{
	if ( initialize() )
	{
		if ( OrgKdeKIMInterface* s = stubForUid( uid ) )
			return s->context( uid );
	}
	return QString();
}

void KIMProxy::chatWithContact( const QString& uid )
{
	if ( initialize() )
	{
		if ( OrgKdeKIMInterface* s = stubForUid( uid ) )
		{
			kapp->updateRemoteUserTimestamp( s->service() );
			s->chatWithContact( uid );
		}
	}
	return;
}

void KIMProxy::messageContact( const QString& uid, const QString& message )
{
	if ( initialize() )
	{
		if ( OrgKdeKIMInterface* s = stubForUid( uid ) )
		{
			kapp->updateRemoteUserTimestamp( s->service() );
			s->messageContact( uid, message );
		}
	}
	return;
}

void KIMProxy::sendFile(const QString &uid, const QString &sourceURL, const QString &altFileName, uint fileSize )
{
	if ( initialize() )
	{
		QHashIterator<QString,OrgKdeKIMInterface*> it( m_im_client_stubs );
		while ( it.hasNext() )
		{
      it.next();
			if ( it.value()->canReceiveFiles( uid ) )
			{
				kapp->updateRemoteUserTimestamp( it.value()->service() );
				it.value()->sendFile( uid, sourceURL, altFileName, fileSize );
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
		if ( OrgKdeKIMInterface* s = stubForProtocol( protocol ) )
		return s->addContact( contactId, protocol );
	}
	return false;
}

QString KIMProxy::locate( const QString & contactId, const QString & protocol )
{
	if ( initialize() )
	{
		if ( OrgKdeKIMInterface* s = stubForProtocol( protocol ) )
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
#ifdef __GNUC__
# warning "unused variable: preferences"
#endif
	QString preferences = QString("[X-DBUS-ServiceName] = '%1'").arg( preferredApp() );
	// start/find an instance of DBUS/InstantMessenger
	QString error;
	QString dbusService;
	// Get a preferred IM client.
	// The app will notify itself to us using nameOwnerChanged, so we don't need to record a stub for it here
	// FIXME: error in preferences, see debug output
	preferences.clear();
	int result = KDBusServiceStarter::self()->findServiceFor( IM_SERVICE_TYPE, QString("Application"), &error, &dbusService );

	kDebug( debugArea() ) << "error was: " << error << ", dbusService: " << dbusService;

	return ( result == 0 );
}


void KIMProxy::pollAll( const QString &uid )
{
	Q_UNUSED(uid);
/*	// We only need to call this function if we don't have any data at all
	// otherwise, the data will be kept fresh by received presence change
	// DCOP signals
	if ( !d->presence_map.contains( uid ) )
	{
		AppPresence *presence = new AppPresence();
		// record current presence from known clients
		QDictIterator<OrgKdeKIMInterface> it( m_im_client_stubs );
		for ( ; it.current(); ++it )
		{
			presence->insert( it.currentKey().toAscii().constData(), it.current()->presenceStatus( uid ) ); // m_im_client_stubs has qstring keys...
		}
		d->presence_map.insert( uid, presence );
	}*/
}

void KIMProxy::pollApp( const QString & appId )
{
	//kDebug( debugArea() ) ;
	OrgKdeKIMInterface * appStub = m_im_client_stubs.value( appId );
	QStringList contacts = m_im_client_stubs.value( appId )->allContacts();
	QStringList::iterator it = contacts.begin();
	QStringList::iterator end = contacts.end();
	for ( ; it != end; ++it )
	{
		ContactPresenceListCurrent current = d->presence_map[ *it ];
		AppPresenceCurrent ap;
		ap.appId = appId;
#ifdef __GNUC__
# warning "KIMProxy::pollApp( const QString & appId ).presenceStatus() function doesn't exist Need to fix it"
#endif
		//ap.presence = appStub->presenceStatus( *it );
		current.append( ap );

		d->presence_map.insert( *it, current );
		if ( current.update( ap ) )
			emit sigContactPresenceChanged( *it );
		//kDebug( debugArea() ) << " uid: " << *it << " presence: " << ap.presence;
	}
}

OrgKdeKIMInterface * KIMProxy::stubForUid( const QString &uid )
{
	// get best appPresence
	AppPresenceCurrent ap = d->presence_map[ uid ].best();
	// look up the presence string from that app
		return m_im_client_stubs.value( ap.appId );
}

OrgKdeKIMInterface * KIMProxy::stubForProtocol( const QString &protocol)
{
    Q_UNUSED(protocol)
#ifdef __GNUC__
# warning "KIMProxy::stubForProtocol( const QString &protocol) code disabled: protocols() function doesn't exist. Need to fix it"
#endif
#if 0
	OrgKdeKIMInterface * app;
	// see if the preferred client supports this protocol
	QString preferred = preferredApp();
	if ( ( app = m_im_client_stubs.value( preferred ) ) )
	{
		if ( app->protocols().value().filter( protocol ).count() > 0 )
			return app;
	}
	// preferred doesn't do this protocol, try the first of the others that says it does
	QHashIterator<QString, OrgKdeKIMInterface*> it( m_im_client_stubs );
	while ( it.hasNext() )
	{
		it.next();
		if ( it.value()->protocols().value().filter( protocol ).count() > 0 )
			return it.value();
	}
#endif
	return 0L;
}

QString KIMProxy::preferredApp()
{
	KConfig cfg( IM_CLIENT_PREFERENCES_FILE, KConfig::SimpleConfig );
	KConfigGroup cg(&cfg, IM_CLIENT_PREFERENCES_SECTION );
	QString preferredApp = cg.readEntry( IM_CLIENT_PREFERENCES_ENTRY );
	//kDebug( debugArea() ) << "found preferred app: " << preferredApp;
	return preferredApp;
}

#include "kimproxy.moc"

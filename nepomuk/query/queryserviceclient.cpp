/*
   Copyright (c) 2008-2009 Sebastian Trueg <trueg@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) version 3, or any
   later version accepted by the membership of KDE e.V. (or its
   successor approved by the membership of KDE e.V.), which shall
   act as a proxy defined in Section 6 of version 3 of the license.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "queryserviceclient.h"
#include "dbusoperators_p.h"
#include "result.h"
#include "query.h"
#include "queryserviceinterface.h"
#include "queryinterface.h"

#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>
#include <QtDBus/QDBusReply>

#include <QtCore/QEventLoop>
#include <QtCore/QTimer>

#include <kdebug.h>
#include <kglobal.h>


namespace {
    /**
     * Each thread needs its own QDBusConnection. We do it the very easy
     * way and just create a new connection for each client
     * Why does Qt not handle this automatically?
     */
    class QDBusConnectionPerThreadHelper
    {
    public:
        QDBusConnectionPerThreadHelper()
            : m_counter( 0 ) {
        }

        QDBusConnection newConnection() {
            QMutexLocker lock( &m_mutex );
            return QDBusConnection::connectToBus( QDBusConnection::SessionBus,
                                                  QString("NepomukQueryServiceConnection%1").arg(++m_counter) );
        }

    private:
        int m_counter;
        QMutex m_mutex;
    };

    K_GLOBAL_STATIC( QDBusConnectionPerThreadHelper, s_globalDBusConnectionPerThreadHelper )

    /// create a mapping from binding name to property
    QHash<QString, QString> encodeRequestProperties( const QList<Nepomuk::Query::Query::RequestProperty>& rps )
    {
        QHash<QString, QString> encodedRps;
        int i = 1;
        foreach( const Nepomuk::Query::Query::RequestProperty& rp, rps ) {
            encodedRps.insert( QString( "reqProp%1" ).arg( i++ ), KUrl( rp.property().uri() ).url() );
        }
        return encodedRps;
    }

    /// create a mapping from binding name to property
    QHash<QString, QString> encodeRequestProperties( const QHash<QString, Nepomuk::Types::Property>& rps )
    {
        QHash<QString, QString> encodedRps;
        for( QHash<QString, Nepomuk::Types::Property>::const_iterator it = rps.constBegin();
             it != rps.constEnd(); ++it ) {
            encodedRps.insert( it.key(), KUrl( it.value().uri() ).url() );
        }
        return encodedRps;
    }
}


class Nepomuk::Query::QueryServiceClient::Private
{
public:
    Private()
        : queryServiceInterface( 0 ),
          queryInterface( 0 ),
          dbusConnection( s_globalDBusConnectionPerThreadHelper->newConnection() ),
          loop( 0 ) {
    }

    void _k_entriesRemoved( const QStringList& );
    void _k_finishedListing();
    bool handleQueryReply( QDBusReply<QDBusObjectPath> reply );

    org::kde::nepomuk::QueryService* queryServiceInterface;
    org::kde::nepomuk::Query* queryInterface;

    QueryServiceClient* q;

    QDBusConnection dbusConnection;

    QEventLoop* loop;
};


void Nepomuk::Query::QueryServiceClient::Private::_k_entriesRemoved( const QStringList& uris )
{
    QList<QUrl> ul;
    foreach( const QString& s, uris ) {
        ul.append( QUrl( s ) );
    }
    emit q->entriesRemoved( ul );
}


void Nepomuk::Query::QueryServiceClient::Private::_k_finishedListing()
{
    emit q->finishedListing();
    if( loop ) {
        q->close();
    }
}


bool Nepomuk::Query::QueryServiceClient::Private::handleQueryReply( QDBusReply<QDBusObjectPath> r )
{
    if ( r.isValid() ) {
        queryInterface = new org::kde::nepomuk::Query( queryServiceInterface->service(),
                                                       r.value().path(),
                                                       dbusConnection  );
        connect( queryInterface, SIGNAL( newEntries( QList<Nepomuk::Query::Result> ) ),
                 q, SIGNAL( newEntries( QList<Nepomuk::Query::Result> ) ) );
        connect( queryInterface, SIGNAL( entriesRemoved( QStringList ) ),
                 q, SLOT( _k_entriesRemoved( QStringList ) ) );
        connect( queryInterface, SIGNAL( finishedListing() ),
                 q, SLOT( _k_finishedListing() ) );
        // run the listing async in case the event loop below is the only one we have
        // and we need it to handle the signals and list returns results immediately
        QTimer::singleShot( 0, queryInterface, SLOT(list()) );
        return true;
    }
    else {
        kDebug() << "Query failed:" << r.error().message();
        return false;
    }
}


Nepomuk::Query::QueryServiceClient::QueryServiceClient( QObject* parent )
    : QObject( parent ),
      d( new Private() )
{
    d->q = this;

    Nepomuk::Query::registerDBusTypes();

    // we use our own connection to be thread-safe
    d->queryServiceInterface = new org::kde::nepomuk::QueryService( "org.kde.nepomuk.services.nepomukqueryservice",
                                                                    "/nepomukqueryservice",
                                                                    d->dbusConnection );
}


Nepomuk::Query::QueryServiceClient::~QueryServiceClient()
{
    close();
    delete d;
}


bool Nepomuk::Query::QueryServiceClient::query( const Query& query )
{
    close();

    if ( d->queryServiceInterface->isValid() ) {
        return d->handleQueryReply( d->queryServiceInterface->sparqlQuery( query.toSparqlQuery(), encodeRequestProperties( query.requestProperties() ) ) );
    }
    else {
        kDebug() << "Could not contact query service.";
        return false;
    }
}


bool Nepomuk::Query::QueryServiceClient::sparqlQuery( const QString& query, const QHash<QString, Nepomuk::Types::Property>& requestPropertyMap )
{
    close();

    if ( d->queryServiceInterface->isValid() ) {
        return d->handleQueryReply( d->queryServiceInterface->sparqlQuery( query, encodeRequestProperties( requestPropertyMap ) ) );
    }
    else {
        kDebug() << "Could not contact query service.";
        return false;
    }
}


bool Nepomuk::Query::QueryServiceClient::desktopQuery( const QString& query )
{
    close();

    if ( d->queryServiceInterface->isValid() ) {
        return d->handleQueryReply( d->queryServiceInterface->query( query ) );
    }
    else {
        kDebug() << "Could not contact query service.";
        return false;
    }
}


bool Nepomuk::Query::QueryServiceClient::blockingQuery( const Query& q )
{
    if( query( q ) ) {
        QEventLoop loop;
        d->loop = &loop;
        loop.exec();
        d->loop = 0;
        return true;
    }
    else {
        return false;
    }
}


bool Nepomuk::Query::QueryServiceClient::blockingSparqlQuery( const QString& q, const QHash<QString, Nepomuk::Types::Property>& requestPropertyMap )
{
    if( sparqlQuery( q, requestPropertyMap ) ) {
        QEventLoop loop;
        d->loop = &loop;
        loop.exec();
        d->loop = 0;
        return true;
    }
    else {
        return false;
    }
}


bool Nepomuk::Query::QueryServiceClient::blockingDesktopQuery( const QString& q )
{
    if( desktopQuery( q ) ) {
        QEventLoop loop;
        d->loop = &loop;
        loop.exec();
        d->loop = 0;
        return true;
    }
    else {
        return false;
    }
}


void Nepomuk::Query::QueryServiceClient::close()
{
    if ( d->queryInterface ) {
        kDebug();
        d->queryInterface->close();
        delete d->queryInterface;
        d->queryInterface = 0;
        if( d->loop )
            d->loop->exit();
    }
}


bool Nepomuk::Query::QueryServiceClient::serviceAvailable()
{
    return QDBusConnection::sessionBus().interface()->isServiceRegistered( "org.kde.nepomuk.services.nepomukqueryservice" );
}

#include "queryserviceclient.moc"

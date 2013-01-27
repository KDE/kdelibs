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
#include "queryserviceclient_p.h"
#include "dbusoperators_p.h"
#include "result.h"
#include "query.h"
#include "queryserviceinterface.h"
#include "queryinterface.h"
#include <dbusconnectionpool.h>

#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>
#include <QtDBus/QDBusReply>

#include <QtCore/QEventLoop>
#include <QtCore/QTimer>

#include <kdebug.h>
#include <kglobal.h>


namespace {
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

    NepomukResultListEventLoop::NepomukResultListEventLoop(Nepomuk::Query::QueryServiceClient* parent)
        : QEventLoop(parent)
    {
        connect(parent, SIGNAL(newEntries(QList<Nepomuk::Query::Result>)),
                this, SLOT(addEntries(QList<Nepomuk::Query::Result>)));
    }

    NepomukResultListEventLoop::~NepomukResultListEventLoop()
    {
    }

    void NepomukResultListEventLoop::addEntries(const QList< Nepomuk::Query::Result >& entries)
    {
        m_result << entries;
    }

    QList< Nepomuk::Query::Result > NepomukResultListEventLoop::result() const
    {
        return m_result;
    }

}


class Nepomuk::Query::QueryServiceClient::Private
{
public:
    Private()
        : queryServiceInterface( 0 ),
          queryInterface( 0 ),
          dbusConnection( DBusConnectionPool::threadConnection() ),
          m_queryActive( false ),
          loop( 0 ) {
    }

    void _k_entriesRemoved( const QStringList& );
    void _k_finishedListing();
    void _k_handleQueryReply(QDBusPendingCallWatcher*);
    void _k_serviceRegistered( const QString& );
    void _k_serviceUnregistered( const QString& );

    org::kde::nepomuk::QueryService* queryServiceInterface;
    org::kde::nepomuk::Query* queryInterface;
    QDBusServiceWatcher *queryServiceWatcher;

    QueryServiceClient* q;

    QPointer<QDBusPendingCallWatcher> m_pendingCallWatcher;

    QDBusConnection dbusConnection;

    bool m_queryActive;
    QEventLoop* loop;
    QString m_errorMessage;
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
    m_queryActive = false;
    emit q->finishedListing();
    if( loop ) {
        q->close();
    }
}


void Nepomuk::Query::QueryServiceClient::Private::_k_handleQueryReply(QDBusPendingCallWatcher* watcher)
{
    QDBusPendingReply<QDBusObjectPath> reply = *watcher;
    if(reply.isError()) {
        kDebug() << reply.error();
        m_errorMessage = reply.error().message();
        m_queryActive = false;
        emit q->error(m_errorMessage);
        if( loop ) {
            loop->exit();
        }
    }
    else {
        queryInterface = new org::kde::nepomuk::Query( queryServiceInterface->service(),
                                                       reply.value().path(),
                                                       dbusConnection );
        connect( queryInterface, SIGNAL(newEntries(QList<Nepomuk::Query::Result>)),
                 q, SIGNAL(newEntries(QList<Nepomuk::Query::Result>)) );
        connect( queryInterface, SIGNAL(resultCount(int)),
                 q, SIGNAL(resultCount(int)) );
        connect( queryInterface, SIGNAL(entriesRemoved(QStringList)),
                 q, SLOT(_k_entriesRemoved(QStringList)) );
        connect( queryInterface, SIGNAL(finishedListing()),
                 q, SLOT(_k_finishedListing()) );
        // run the listing async in case the event loop below is the only one we have
        // and we need it to handle the signals and list returns results immediately
        QTimer::singleShot( 0, queryInterface, SLOT(list()) );
    }

    delete watcher;
}


void Nepomuk::Query::QueryServiceClient::Private::_k_serviceRegistered(const QString &service)
{
    if (service == "org.kde.nepomuk.services.nepomukqueryservice") {
        delete queryServiceInterface;
        queryServiceInterface = new org::kde::nepomuk::QueryService( "org.kde.nepomuk.services.nepomukqueryservice",
                                                                        "/nepomukqueryservice",
                                                                        dbusConnection );
        emit q->serviceAvailabilityChanged(true);
    }
}


void Nepomuk::Query::QueryServiceClient::Private::_k_serviceUnregistered(const QString &service)
{
    if (service == "org.kde.nepomuk.services.nepomukqueryservice") {
        emit q->serviceAvailabilityChanged(false);
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
    d->queryServiceWatcher = new QDBusServiceWatcher(QLatin1String("org.kde.nepomuk.services.nepomukqueryservice"),
                                                     QDBusConnection::sessionBus(),
                                                     QDBusServiceWatcher::WatchForOwnerChange,
                                                     this);
    connect(d->queryServiceWatcher, SIGNAL(serviceRegistered(QString)), this, SLOT(_k_serviceRegistered(QString)));
    connect(d->queryServiceWatcher, SIGNAL(serviceUnregistered(QString)), this, SLOT(_k_serviceUnregistered(QString)));
}


Nepomuk::Query::QueryServiceClient::~QueryServiceClient()
{
    close();
    delete d->queryServiceInterface;
    delete d;
}


bool Nepomuk::Query::QueryServiceClient::query( const Query& query )
{
    close();

    if ( d->queryServiceInterface->isValid() ) {
        d->m_queryActive = true;
        d->m_pendingCallWatcher = new QDBusPendingCallWatcher(d->queryServiceInterface->asyncCall(QLatin1String("query"),
                                                                                                  query.toString()),
                                                              this);
        connect(d->m_pendingCallWatcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                this, SLOT(_k_handleQueryReply(QDBusPendingCallWatcher*)));
        return true;
    }
    else {
        kDebug() << "Could not contact nepomuk query service.";
        return false;
    }
}


bool Nepomuk::Query::QueryServiceClient::sparqlQuery( const QString& query, const QHash<QString, Nepomuk::Types::Property>& requestPropertyMap )
{
    close();

    if ( d->queryServiceInterface->isValid() ) {
        d->m_queryActive = true;
        d->m_pendingCallWatcher = new QDBusPendingCallWatcher(d->queryServiceInterface->asyncCall(QLatin1String("sparqlQuery"),
                                                                                                  query,
                                                                                                  QVariant::fromValue(encodeRequestProperties( requestPropertyMap ))),
                                                              this);
        connect(d->m_pendingCallWatcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                this, SLOT(_k_handleQueryReply(QDBusPendingCallWatcher*)));
        return true;
    }
    else {
        kDebug() << "Could not contact nepomuk query service.";
        return false;
    }
}


bool Nepomuk::Query::QueryServiceClient::desktopQuery( const QString& query )
{
    close();

    if ( d->queryServiceInterface->isValid() ) {
        d->m_queryActive = true;
        d->m_pendingCallWatcher = new QDBusPendingCallWatcher(d->queryServiceInterface->asyncCall(QLatin1String("desktopQuery"),
                                                                                                  query),
                                                              this);
        connect(d->m_pendingCallWatcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                this, SLOT(_k_handleQueryReply(QDBusPendingCallWatcher*)));
        return true;
    }
    else {
        kDebug() << "Could not contact nepomuk query service.";
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
        close();
        return true;
    }
    else {
        return false;
    }
}


QList< Nepomuk::Query::Result > Nepomuk::Query::QueryServiceClient::syncQuery(const Query& q, bool* ok)
{
    QueryServiceClient qsc;
    if( qsc.query( q ) ) {
        NepomukResultListEventLoop loop(&qsc);
        qsc.d->loop = &loop;
        loop.exec();
        qsc.d->loop = 0;
        if (ok) {
            *ok = !qsc.errorMessage().isEmpty();
        }
        return loop.result();
    }
    else {
        if (ok) {
            *ok = false;
        }
        return QList< Nepomuk::Query::Result >();
    }
}


bool Nepomuk::Query::QueryServiceClient::blockingSparqlQuery( const QString& q, const QHash<QString, Nepomuk::Types::Property>& requestPropertyMap )
{
    if( sparqlQuery( q, requestPropertyMap ) ) {
        QEventLoop loop;
        d->loop = &loop;
        loop.exec();
        d->loop = 0;
        close();
        return true;
    }
    else {
        return false;
    }
}


QList< Nepomuk::Query::Result > Nepomuk::Query::QueryServiceClient::syncSparqlQuery(const QString& q,
                                                    const QHash<QString, Nepomuk::Types::Property>& requestPropertyMap,
                                                    bool *ok)
{
    QueryServiceClient qsc;
    if( qsc.sparqlQuery( q, requestPropertyMap ) ) {
        NepomukResultListEventLoop loop(&qsc);
        qsc.d->loop = &loop;
        loop.exec();
        qsc.d->loop = 0;
        if (ok) {
            *ok = !qsc.errorMessage().isEmpty();
        }
        return loop.result();
    }
    else {
        if (ok) {
            *ok = false;
        }
        return QList< Nepomuk::Query::Result >();
    }
}


bool Nepomuk::Query::QueryServiceClient::blockingDesktopQuery( const QString& q )
{
    if( desktopQuery( q ) ) {
        QEventLoop loop;
        d->loop = &loop;
        loop.exec();
        d->loop = 0;
        close();
        return true;
    }
    else {
        return false;
    }
}


QList< Nepomuk::Query::Result > Nepomuk::Query::QueryServiceClient::syncDesktopQuery(const QString& q, bool* ok)
{
    QueryServiceClient qsc;
    if( qsc.desktopQuery( q ) ) {
        NepomukResultListEventLoop loop(&qsc);
        qsc.d->loop = &loop;
        loop.exec();
        qsc.d->loop = 0;
        if (ok) {
            *ok = !qsc.errorMessage().isEmpty();
        }
        return loop.result();
    }
    else {
        if (ok) {
            *ok = false;
        }
        return QList< Nepomuk::Query::Result >();
    }
}


void Nepomuk::Query::QueryServiceClient::close()
{
    // drop pending query calls

    // in case we fired a query but it did not return yet, cancel it
    if (d->m_pendingCallWatcher && !d->queryInterface) {
        QDBusPendingReply<QDBusObjectPath> reply = *(d->m_pendingCallWatcher);
        OrgKdeNepomukQueryInterface interface( d->queryServiceInterface->service(),
                                               reply.value().path(),
                                               d->dbusConnection );
        interface.close();
    }
    delete d->m_pendingCallWatcher;

    d->m_errorMessage.truncate(0);

    if ( d->queryInterface ) {
        kDebug();
        d->queryInterface->close();
        delete d->queryInterface;
        d->queryInterface = 0;
        d->m_queryActive = false;
        if( d->loop )
            d->loop->exit();
    }
}


bool Nepomuk::Query::QueryServiceClient::isListingFinished() const
{
    return !d->m_queryActive;
}


bool Nepomuk::Query::QueryServiceClient::serviceAvailable()
{
    return QDBusConnection::sessionBus().interface()->isServiceRegistered( "org.kde.nepomuk.services.nepomukqueryservice" );
}


QString Nepomuk::Query::QueryServiceClient::errorMessage() const
{
    return d->m_errorMessage;
}

#include "queryserviceclient.moc"
#include "queryserviceclient_p.moc"

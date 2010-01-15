/*
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2008 Sebastian Trueg <trueg@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "nepomukmainmodel.h"
#include "resourcemanager.h"

#include <Soprano/Node>
#include <Soprano/Statement>
#include <Soprano/StatementIterator>
#include <Soprano/NodeIterator>
#include <Soprano/QueryResultIterator>
#include <Soprano/Client/DBusModel>
#include <Soprano/Client/DBusClient>
#include <Soprano/Client/LocalSocketClient>
#include <Soprano/Query/QueryLanguage>
#include <Soprano/Util/DummyModel>
#include <Soprano/Util/MutexModel>
#include <Soprano/Vocabulary/RDF>
#include <Soprano/Vocabulary/NRL>
#include <Soprano/Vocabulary/NAO>

#include <kglobal.h>
#include <kstandarddirs.h>
#include <kdebug.h>

#include <QtCore/QTimer>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>


// FIXME: connect to some NepomukServer signal which emit enabled/disabled information
//        when the server shuts down and is started again
// FIXME: disconnect localSocketClient after n seconds of idling (but take care of not
//        disconnecting when iterators are open)

using namespace Soprano;


namespace {
class GlobalModelContainer
{
public:
    GlobalModelContainer()
        : dbusClient( "org.kde.nepomuk.services.nepomukstorage" ),
          dbusModel( 0 ),
          localSocketModel( 0 ),
          mutexModel( 0 ),
          dummyModel( 0 ),
          m_socketConnectFailed( false ) {
    }

    ~GlobalModelContainer() {
        delete mutexModel;
        delete dbusModel;
        delete localSocketModel;
        delete dummyModel;
    }

    Soprano::Client::DBusClient dbusClient;
    Soprano::Client::LocalSocketClient localSocketClient;
    Soprano::Client::DBusModel* dbusModel;
    Soprano::Model* localSocketModel;
    Soprano::Util::MutexModel* mutexModel;

    Soprano::Util::DummyModel* dummyModel;

    void init() {
        QMutexLocker lock( &m_initMutex );

        // Comment out the check for the storage service via DBus - fixes bug 209821
        //if ( QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.NepomukStorage") ) {
            if ( !dbusModel ) {
                dbusModel = dbusClient.createModel( "main" );
            }

            if ( !mutexModel ) {
                mutexModel = new Soprano::Util::MutexModel( Soprano::Util::MutexModel::ReadWriteMultiThreading );
            }

            // we may get disconnected from the server but we don't want to try
            // to connect every time the model is requested
            if ( !m_socketConnectFailed && !localSocketClient.isConnected() ) {
                if ( mutexModel->parentModel() == localSocketModel )
                    mutexModel->setParentModel( 0 );
                delete localSocketModel;
                localSocketModel = 0;
                QString socketName = KGlobal::dirs()->locateLocal( "data", "nepomuk/socket" );
                if ( localSocketClient.connect( socketName ) ) {
                    localSocketModel = localSocketClient.createModel( "main" );
                }
                else {
                    m_socketConnectFailed = true;
                    kDebug() << "Failed to connect to Nepomuk server via local socket" << socketName;
                }
            }
        //}
    }

    Soprano::Model* model() {
        init();

        QMutexLocker lock( &m_initMutex );

        // we always prefer the faster local socket client
        if ( localSocketModel ) {
            if ( mutexModel->parentModel() != localSocketModel ) {
                mutexModel->setParentModel( localSocketModel );
            }
        }
        else if ( dbusModel ) {
            if ( mutexModel->parentModel() != dbusModel ) {
                mutexModel->setParentModel( dbusModel );
            }
        }
        else {
            if ( !dummyModel ) {
                dummyModel = new Soprano::Util::DummyModel();
            }
            return dummyModel;
        }

        return mutexModel;
    }

private:
    bool m_socketConnectFailed;
    QMutex m_initMutex;
};
}

Q_GLOBAL_STATIC( GlobalModelContainer, modelContainer )


class Nepomuk::MainModel::Private
{
public:
    Private( MainModel* p )
        : q(p) {
    }

private:
    MainModel* q;
};


Nepomuk::MainModel::MainModel( QObject* parent )
    : Soprano::Model(),
      d( new Private(this) )
{
    setParent( parent );

    modelContainer()->init();

    if ( modelContainer()->dbusModel ) {
        // we have to use the dbus model for signals in any case
        connect( modelContainer()->dbusModel, SIGNAL( statementsAdded() ),
                 this, SIGNAL( statementsAdded() ) );
        connect( modelContainer()->dbusModel, SIGNAL( statementsRemoved() ),
                 this, SIGNAL( statementsRemoved() ) );
        connect( modelContainer()->dbusModel, SIGNAL( statementAdded(const Soprano::Statement&) ),
                 this, SIGNAL( statementAdded(const Soprano::Statement&) ) );
        connect( modelContainer()->dbusModel, SIGNAL( statementRemoved(const Soprano::Statement&) ),
                 this, SIGNAL( statementRemoved(const Soprano::Statement&) ) );
    }
}


Nepomuk::MainModel::~MainModel()
{
    delete d;
}


bool Nepomuk::MainModel::isValid() const
{
    return modelContainer()->dbusClient.isValid() || modelContainer()->localSocketClient.isConnected();
}


Soprano::StatementIterator Nepomuk::MainModel::listStatements( const Statement& partial ) const
{
    Soprano::StatementIterator it = modelContainer()->model()->listStatements( partial );
    setError( modelContainer()->model()->lastError() );
    return it;
}


Soprano::NodeIterator Nepomuk::MainModel::listContexts() const
{
    Soprano::NodeIterator it = modelContainer()->model()->listContexts();
    setError( modelContainer()->model()->lastError() );
    return it;
}


Soprano::QueryResultIterator Nepomuk::MainModel::executeQuery( const QString& query,
                                                               Soprano::Query::QueryLanguage language,
                                                               const QString& userQueryLanguage ) const
{
    Soprano::QueryResultIterator it = modelContainer()->model()->executeQuery( query, language, userQueryLanguage );
    setError( modelContainer()->model()->lastError() );
    return it;
}


bool Nepomuk::MainModel::containsStatement( const Statement& statement ) const
{
    bool b = modelContainer()->model()->containsStatement( statement );
    setError( modelContainer()->model()->lastError() );
    return b;
}


bool Nepomuk::MainModel::containsAnyStatement( const Statement &statement ) const
{
    bool b = modelContainer()->model()->containsAnyStatement( statement );
    setError( modelContainer()->model()->lastError() );
    return b;
}


bool Nepomuk::MainModel::isEmpty() const
{
    bool b = modelContainer()->model()->isEmpty();
    setError( modelContainer()->model()->lastError() );
    return b;
}


int Nepomuk::MainModel::statementCount() const
{
    int c = modelContainer()->model()->statementCount();
    setError( modelContainer()->model()->lastError() );
    return c;
}


Soprano::Error::ErrorCode Nepomuk::MainModel::addStatement( const Statement& statement )
{
    Soprano::Error::ErrorCode c = modelContainer()->model()->addStatement( statement );
    setError( modelContainer()->model()->lastError() );
    return c;
}


Soprano::Error::ErrorCode Nepomuk::MainModel::removeStatement( const Statement& statement )
{
    Soprano::Error::ErrorCode c = modelContainer()->model()->removeStatement( statement );
    setError( modelContainer()->model()->lastError() );
    return c;
}


Soprano::Error::ErrorCode Nepomuk::MainModel::removeAllStatements( const Statement& statement )
{
    Soprano::Error::ErrorCode c = modelContainer()->model()->removeAllStatements( statement );
    setError( modelContainer()->model()->lastError() );
    return c;
}


Soprano::Node Nepomuk::MainModel::createBlankNode()
{
    Soprano::Node n = modelContainer()->model()->createBlankNode();
    setError( modelContainer()->model()->lastError() );
    return n;
}

#include "nepomukmainmodel.moc"

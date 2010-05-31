/*
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2008-2010 Sebastian Trueg <trueg@kde.org>
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
          dummyModel( 0 ),
          m_socketConnectFailed( false ),
          m_initMutex( QMutex::Recursive ) {
    }

    ~GlobalModelContainer() {
        delete dbusModel;
        delete localSocketModel;
        delete dummyModel;
    }

    Soprano::Client::DBusClient dbusClient;
    Soprano::Client::LocalSocketClient localSocketClient;
    Soprano::Client::DBusModel* dbusModel;
    Soprano::Model* localSocketModel;

    Soprano::Util::DummyModel* dummyModel;

    void init( bool forced ) {
        QMutexLocker lock( &m_initMutex );

        if( forced ) {
            m_socketConnectFailed = false;
            delete dbusModel;
            dbusModel = 0;
        }

        // TODO: check if the service is also initialized
        if ( !dbusModel ) {
            dbusModel = dbusClient.createModel( "main" );
        }

        // we may get disconnected from the server but we don't want to try
        // to connect every time the model is requested
        if ( !m_socketConnectFailed && !localSocketClient.isConnected() ) {
            delete localSocketModel;
            localSocketModel = 0;
            QString socketName = KGlobal::dirs()->locateLocal( "data", "nepomuk/socket" );
            kDebug() << "Connecting to local socket" << socketName;
            if ( localSocketClient.connect( socketName ) ) {
                localSocketModel = localSocketClient.createModel( "main" );
            }
            else {
                m_socketConnectFailed = true;
                kDebug() << "Failed to connect to Nepomuk server via local socket" << socketName;
            }
        }
    }

    Soprano::Model* model() {
        QMutexLocker lock( &m_initMutex );

        init( false );

        // we always prefer the faster local socket client
        if ( localSocketModel ) {
            return localSocketModel;
        }
        else if ( dbusModel ) {
            return dbusModel;
        }
        else {
            if ( !dummyModel ) {
                dummyModel = new Soprano::Util::DummyModel();
            }
            return dummyModel;
        }
    }

private:
    bool m_socketConnectFailed;
    QMutex m_initMutex;
};
}

K_GLOBAL_STATIC( GlobalModelContainer, s_modelContainer )


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
    init();
}


Nepomuk::MainModel::~MainModel()
{
    delete d;
}


bool Nepomuk::MainModel::isValid() const
{
    return s_modelContainer->dbusClient.isValid() || s_modelContainer->localSocketClient.isConnected();
}


bool Nepomuk::MainModel::init()
{
    s_modelContainer->init( true );
    if ( s_modelContainer->dbusModel ) {
        // we have to use the dbus model for signals in any case
        connect( s_modelContainer->dbusModel, SIGNAL( statementsAdded() ),
                 this, SIGNAL( statementsAdded() ) );
        connect( s_modelContainer->dbusModel, SIGNAL( statementsRemoved() ),
                 this, SIGNAL( statementsRemoved() ) );
        connect( s_modelContainer->dbusModel, SIGNAL( statementAdded(const Soprano::Statement&) ),
                 this, SIGNAL( statementAdded(const Soprano::Statement&) ) );
        connect( s_modelContainer->dbusModel, SIGNAL( statementRemoved(const Soprano::Statement&) ),
                 this, SIGNAL( statementRemoved(const Soprano::Statement&) ) );
    }
    return isValid();
}


Soprano::StatementIterator Nepomuk::MainModel::listStatements( const Statement& partial ) const
{
    Soprano::StatementIterator it = s_modelContainer->model()->listStatements( partial );
    setError( s_modelContainer->model()->lastError() );
    return it;
}


Soprano::NodeIterator Nepomuk::MainModel::listContexts() const
{
    Soprano::NodeIterator it = s_modelContainer->model()->listContexts();
    setError( s_modelContainer->model()->lastError() );
    return it;
}


Soprano::QueryResultIterator Nepomuk::MainModel::executeQuery( const QString& query,
                                                               Soprano::Query::QueryLanguage language,
                                                               const QString& userQueryLanguage ) const
{
    Soprano::QueryResultIterator it = s_modelContainer->model()->executeQuery( query, language, userQueryLanguage );
    setError( s_modelContainer->model()->lastError() );
    return it;
}


bool Nepomuk::MainModel::containsStatement( const Statement& statement ) const
{
    bool b = s_modelContainer->model()->containsStatement( statement );
    setError( s_modelContainer->model()->lastError() );
    return b;
}


bool Nepomuk::MainModel::containsAnyStatement( const Statement &statement ) const
{
    bool b = s_modelContainer->model()->containsAnyStatement( statement );
    setError( s_modelContainer->model()->lastError() );
    return b;
}


bool Nepomuk::MainModel::isEmpty() const
{
    bool b = s_modelContainer->model()->isEmpty();
    setError( s_modelContainer->model()->lastError() );
    return b;
}


int Nepomuk::MainModel::statementCount() const
{
    int c = s_modelContainer->model()->statementCount();
    setError( s_modelContainer->model()->lastError() );
    return c;
}


Soprano::Error::ErrorCode Nepomuk::MainModel::addStatement( const Statement& statement )
{
    Soprano::Error::ErrorCode c = s_modelContainer->model()->addStatement( statement );
    setError( s_modelContainer->model()->lastError() );
    return c;
}


Soprano::Error::ErrorCode Nepomuk::MainModel::removeStatement( const Statement& statement )
{
    Soprano::Error::ErrorCode c = s_modelContainer->model()->removeStatement( statement );
    setError( s_modelContainer->model()->lastError() );
    return c;
}


Soprano::Error::ErrorCode Nepomuk::MainModel::removeAllStatements( const Statement& statement )
{
    Soprano::Error::ErrorCode c = s_modelContainer->model()->removeAllStatements( statement );
    setError( s_modelContainer->model()->lastError() );
    return c;
}


Soprano::Node Nepomuk::MainModel::createBlankNode()
{
    Soprano::Node n = s_modelContainer->model()->createBlankNode();
    setError( s_modelContainer->model()->lastError() );
    return n;
}

#include "nepomukmainmodel.moc"

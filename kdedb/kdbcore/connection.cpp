/*
   This file is part of the KDB libraries
   Copyright (c) 2000 Praduroux Alessandro <pradu@thekompany.com>
 
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
#include <kpassdlg.h>
#include <klocale.h>

#include "dbengine.h"
#include "connection.h"
#include "database.h"
#include "connector.h"

#include <kdebug.h>
#include <kconfigbase.h>

#include "connection.moc"

using namespace KDB;

Connection::Connection( const QString &plugin, const QString &host, int port, QObject *parent )
    :Object( parent, "" ), connector(0), m_plugin( plugin ), m_host(host), m_port(port), m_savePwd(false)
{
    //kdDebug(20000) << "Connection::Connection " << plugin << " " << host << " " << port << endl;
    m_databases.setAutoDelete(true);
}


Connection::~Connection()
{
    //    kdDebug(20000) << "Connection::~Connection" << endl;
    if (connector)
        close();
}

bool
Connection::isConnected()
{
    return (connector != 0L);
    //    return connector->isConnected();
}


void
Connection::setUser(const QString &user)
{
    m_user = user;
}


void
Connection::setPassword(const QString &pwd, bool save)
{
    m_pwd = pwd;
    m_savePwd = save;
}


QString
Connection::password()
{
    return m_pwd;
}

bool
Connection::open()
{
    QCString pwd;

    if (connector) {
        close(false);
    }

    Plugin * plugin = DBENGINE->findPlugin(m_plugin);

    if (!plugin)
        return false;
    
    plugin->registerConnection(this);
    connector = plugin->createConnector();

    connector->setHost(m_host);
    connector->setPort(m_port);
    connector->setUser(m_user);
    connector->setPassword(m_pwd);
    
    if (connector->user().isEmpty()) {
        pushError(new InvalidLogin("No user supplied")); //no connection if the user is unknown
        delete connector;
        connector = 0L;
        return false;
    }

    
    connector->connect();
    if (error()) {
        // we got an invalid login, perhaps the database needs a password?
        if (connector->password().isNull()) {
            clearErrors();
            // ask for the password and try to connect again. if we fail, well throw the exception
            KPasswordDialog::getPassword(pwd,i18n("Please enter password for user %1 at host %2 (%3)").arg(connector->user()).arg(connector->host()).arg(m_plugin));
            setPassword(pwd);
            connector->setPassword(pwd);
            connector->connect();
        }
    }

    // if there is an error in connection return false
    if ( error() ) {
        delete connector;
        connector = 0L;
        return false;
    }
    
    //load all databases (can we simply load names?)
    //kdDebug(20000) << "loading all databases" << endl;
    QStringList dbs = connector->databases();
    for ( QStringList::Iterator it = dbs.begin(); it != dbs.end(); ++it ) {
        Database * base = new Database(connector, this, (*it).utf8());
        m_databases.append(base);
    }

    emit opened();
    return true;
    
}


QString
Connection::host()
{
    return m_host;
}

int
Connection::port()
{
    return m_port;
}

QString
Connection::prettyPrint()
{
    QString ret = name();
    if (!ret.isEmpty()) 
        ret += ": ";
    
    if (m_port != 0) {
        ret += QString("sql://%1@%2:%3/%4").arg(m_user).arg(host()).arg(m_port).arg(m_plugin);
    } else {
        ret += QString("sql://%1@%2/%3").arg(m_user).arg(host()).arg(m_plugin);
    }
    
    return ret;
}

Database *
Connection::openDatabase(const QString &name) 
{
    clearErrors();
    Database * d = findDatabase(name);
    
    if (d && !d->isOpen())
        d->open();

    if (error())
        return 0L;
    
    return d;
}

Database *
Connection::findDatabase(const QString &name) 
{
    DatabaseIterator it = begin();

    while ( it.current() ) {
        if ( it.current()->name() == name )
            break;
        ++it;
    }

    return it.current();
}

DatabaseList
Connection::databases()
{
    return m_databases;
}

DatabaseIterator
Connection::begin()
{
    DatabaseIterator it(m_databases);
    return it;
}
 
/**
  * disconnects from the host. all databases are closed.
  */
void
Connection::close(bool destructive)
{
    //kdDebug(20000) << k_funcinfo << endl;
 
    if (connector) {
        // close all databases.
        for (DatabaseIterator it = begin(); it.current(); ++it) {
            it.current()->close();
        }
        connector->close();
        delete connector;
        connector = 0;
        m_databases.clear();
    }
    if (destructive)
        emit closing(this);
}
 
Database *
Connection::createDatabase(const QString &name) 
{
    if (!connector) {
        pushError(new InvalidRequest("not connected"));
        return 0L;
    }
    connector->createDatabase(name);
    if (error()) {
        return 0L;
    }
    Database * base = new Database(connector, this, name.utf8());
    m_databases.append(base);
    emit databaseAdded(name);
    return base;
}

QString
Connection::user()
{
    return m_user;
}

QString
Connection::plugin()
{
    return m_plugin;
}

bool 
Connection::saveStatus()
{
    return m_saveStatus;
}

void 
Connection::setSaveStatus(bool status)
{
    m_saveStatus = status;
}

void
Connection::saveToConfig(KConfigBase *config, int num)
{
    QString prefix;
    
    if (num != -1) 
        prefix = QString( "%1_" ).arg( num );
    
    config->writeEntry( prefix + "Plugin", m_plugin );
    config->writeEntry( prefix + "Host", m_host );
    config->writeEntry( prefix + "Port", m_port );
    config->writeEntry( prefix + "User", m_user );

    if (m_savePwd)
        config->writeEntry( prefix + "Password", m_pwd );

    if ( !QString(name()).isEmpty() )
        config->writeEntry( prefix + "Name", name() );
}
    

bool
Connection::dropDatabase(const QString &name) 
{
    Database *base = findDatabase(name);

    if (!base)
        return false;

    // TODO drop the database here.

    emit databaseRemoved(name);
    return true;
}


void
Connection::slotDeleteYourself()
{
    //kdDebug(20000) << "Connection::slotDeleteYourself" <<  endl;
    delete this;
}





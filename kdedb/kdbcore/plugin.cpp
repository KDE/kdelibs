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
#include "plugin.h"
#include "connection.h"
#include "dbengine.h"

#include <qtimer.h>

#include "plugin.moc"

using namespace KDB;

Plugin::Plugin(QObject *parent, const char *name)
    :Object( parent, name )
{
    //kdDebug(20012) << "Plugin::Plugin" << endl;
}

Plugin::~Plugin()
{
    kdDebug(20012) << "Plugin::~Plugin : " << m_connections.count() << " connections left to close" << endl;

    ConnectionIterator it = begin();

    while( it.current() ) {
        Connection * conn = it.current();
        bool named = (strlen(conn->name()) != 0);

        kdDebug(20012) << "deleting connection " << conn->prettyPrint() << " named=" << named << endl;
        conn->close();

        // named connection aren't deleted by the close mechanism
        // it is safe to delete them now since the plugin can be deleted only if
        // 1- the app is closing, so deleting connections don't hurt
        // 2- there are no more connections left for this plugin, so we won't get there.
        if (named) { 
            delete conn;
        }
        ++it;
    }
}

Connection *
Plugin::openConnection (const QString &host,
                        int port,
                        const QString &user ,
                        const QString &password )
{
    //kdDebug(20012) << "Plugin::openConnection" << " host=" << host << " port=" << port << " user=" << user << " password=" << password << endl;
    
    Connection * conn = findConnection(host, user);

    if ( !conn )  {
        conn = DBENGINE->addConnection(name(), host, port, user, password);
    }

    if (!conn->isConnected()) {
        conn->open();
    }

    return conn;
}

Connection *
Plugin::addConnection(const QString &host,
                      int port,
                      const QString &user ,
                      const QString &password )
{
    //kdDebug(20012) << "Plugin::addConnection" << " host=" << host << " port=" << port << " user=" << user << " password=" << password << endl;
    
    Connection * conn = DBENGINE->addConnection(name(), host, port);

    if (conn) {
        conn->setUser(user);
        conn->setPassword(password);
    }

    return conn;
}

void
Plugin::remove(Connection * conn)
{
    //kdDebug(20012) << "Plugin::remove" << " conn=" << conn << endl;
    DBENGINE->remove(conn);
}

Connection *
Plugin::findConnection(const QString& host, const QString &user)
{
    //kdDebug(20012) << "Plugin::findConnection" << " host=" << host << " user=" << user << endl;
    ConnectionIterator it = begin();

    while ( it.current() ) {
        if (it.current()->host() == host && it.current()->user() == user) {
            //kdDebug(20012) << "found host " << it.current()->host() << endl;
            break;
        }
        ++it;
    }

    return it.current();
}

ConnectionIterator
Plugin::begin()
{
    //kdDebug(20012) << "Plugin::begin" << endl;
    ConnectionIterator it(m_connections);
    return it;
}

void
Plugin::registerConnection(Connection *conn)
{
    //kdDebug(20012) << "Plugin::registerConnection" << " conn=" << conn << endl;
    if (m_connections.containsRef(conn))
        return;
    m_connections.append(conn);
    connect(conn, SIGNAL(closing(KDB::Connection *)), SLOT(slotConnectionClose(KDB::Connection *)));
}

void
Plugin::slotConnectionClose(Connection *conn)
{
    if (strlen(conn->name()) != 0) {
        // do nothing, named connections must be preserved
        return;
    }

    kdDebug(20012) << "Plugin::slotConnectionClose" << " conn=" << conn << endl;
    m_connections.remove(conn);
    kdDebug(20012) << "connections left: " << m_connections.count() << endl;
    if (m_connections.count() == 0) {
        kdDebug(20012) << "Plugin: no more connections left: closing" << endl;
        emit closing(this);
    }
}

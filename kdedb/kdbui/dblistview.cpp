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
#include "dblistview.h"

#include <kdb/connection.h>
#include <kdb/dbengine.h>
#include <kdb/database.h>
#include <kdb/plugin.h>

#include <klocale.h>
#include <kconfig.h>
#include <kstddirs.h>
#include <kiconloader.h>

#include "dblistview.moc"

using namespace KDB;

DBListViewItem::DBListViewItem(QListView * parent, Object *obj)
    :QListViewItem(parent, obj->name()), m_obj(obj)
{
    init();
}


DBListViewItem::DBListViewItem(QListViewItem * parent, Object *obj)
    :QListViewItem(parent, obj->name()), m_obj(obj)
{
    init();
}


void
DBListViewItem::init()
{
    kdDebug(20001) << m_obj->className() << endl;
    setOpen(true);
    if (m_obj->inherits("KDB::Plugin")) {
        setPixmap(0,BarIcon("plugin"));
    }
    if (m_obj->inherits("KDB::Connection")) {
        setPixmap(0,BarIcon("connection"));
        Connection *conn = static_cast<Connection *>(m_obj);
        setText(0,conn->prettyPrint());
    }
    if (m_obj->inherits("KDB::Database")) {
        setPixmap(0,BarIcon("database"));
    }
        
}

Object*
DBListViewItem::itemObject()
{
    return m_obj;
}


DBListView::DBListView(QWidget *parent, const char * name, bool displayTree, int mode)
    :QListView(parent, name), m_tree( displayTree ), m_mode( mode ), currentDB(0)
{
    addColumn(i18n("Connections/Databases"));
    setColumnWidthMode(0,QListView::Maximum);
    setRootIsDecorated(true);

    if ( m_mode == Auto ) {
            // load all plugins
        for ( PluginIterator itp = DBENGINE->beginPlugins(); itp.current(); ++itp ) {

            DBListViewItem *ip = 0L;

            if (m_tree)
                ip = new DBListViewItem(this, itp.current());
            
        }
            // foreach plugin, load all connections
        for (ConnectionIterator itc = DBENGINE->beginConnections(); itc.current(); ++itc) {
            addConnection(itc.current());
        }
    }

    connect( this,
             SIGNAL( selectionChanged( QListViewItem * ) ),
             SLOT( slotItemSelected( QListViewItem * ) ) );

}
    
DBListView::~DBListView()
{
}

Database *
DBListView::selectedDatabase()
{
    return currentDB;
}

DBListViewItem *
DBListView::addConnection(Connection *connection)
{
    kdDebug(20001) << "DBListView::addConnection 1 : " << connection->plugin() << endl;

    // find plugin entry for this connection
    
    QString pluginName = connection->plugin();
    DBListViewItem *parent = 0L;

    DBListViewItem *item = static_cast<DBListViewItem *>( firstChild() );
    
    while ( item )
    {
        if ( item->text( 0 ) == connection->plugin() )
	    parent = item;
	
	item = static_cast<DBListViewItem *>( item->nextSibling() );
    }

    // add connection to tree
    
    return addConnection(connection, parent);
}

DBListViewItem *
DBListView::addConnection(Connection *conn, DBListViewItem *ip)
{
    kdDebug(20001) << "DBListView::addConnection 2" << endl;

    DBListViewItem *ic = 0L;

    if (m_tree) {
        if (ip) {
            ic = new DBListViewItem( ip, conn );
        } else {
            ic = new DBListViewItem( this, conn );
        }
    }

    // foreach connection, load all databases
    if ( ( m_mode == Auto ) || ( m_mode == SemiAuto ) ) {
        conn->open();
        for (DatabaseIterator itd = conn->begin(); itd.current(); ++itd) {
            if (ic) {
                (void) new DBListViewItem( ic, itd.current());
            } else {
                (void) new DBListViewItem( this, itd.current());
            }
	}
    }

    return ic;    
}

DBListViewItem *
DBListView::addPlugin(Plugin *plugin)
{
    kdDebug(20001) << "DBListView::addPlugin" << endl;

    return new DBListViewItem( this, plugin );
}

DBListViewItem *
DBListView::addDatabase(Database *database)
{
    kdDebug(20001) << "DBListView::addDatabase" << endl;

    return new DBListViewItem( this, database );
}

void 
DBListView::slotItemSelected(QListViewItem *i)
{
    DBListViewItem *v = static_cast<DBListViewItem *>(i);
    if (v->itemObject()->inherits("KDB::Database")) {
        currentDB = static_cast<Database *>(v->itemObject());
        emit databaseSelected(currentDB);
    }
    else {
        currentDB = 0L;
        emit nonDatabaseSelected();
    }
}
